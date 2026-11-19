#include <raylib.h>
#include <ladlib/array.h>
#include "include/particle/particle.h"

#define UNIFORMGRID_LOGGER
#include "include/uniform_grid/uniform_grid.h"

#define SCREEN_WIDTH 1800
#define SCREEN_HEIGHT 1000
#define RADIUS 12
#define CELL_SIZE (RADIUS * 3)

#define FPS 60
#define DT (1.f / FPS)
#define SUB_STEPS 12
#define SDT (DT / SUB_STEPS)
#define FRICTION 0.1
#define RESTITUTION 0.75

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "qhash2d");
    SetTargetFPS(FPS);

    Particle *particles = array_new(sizeof(Particle));
    if (!particles) { CloseWindow(); return 1; }

    UniformGrid *grid = uniformgrid_init(SCREEN_WIDTH, SCREEN_HEIGHT, CELL_SIZE);
    if (!grid) { CloseWindow(); array_destroy((void **) &particles); return 1; }

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) {
            particle_spawn_random(&particles, 300, RADIUS, (Vector2) {0, 980.f}, SCREEN_WIDTH, SCREEN_HEIGHT);
        }

        if (IsKeyPressed(KEY_J)) {
            Vector2 mouse_pos = GetMousePosition();
            particle_spawn_burst(&particles, mouse_pos, 50, RADIUS, (Vector2){0, 980.f});
        }

        for (int i = 0; i < SUB_STEPS; i++) {

            particle_update(particles, SDT);
            // particle_constrain(particles, RESTITUTION, FRICTION, SCREEN_WIDTH, SCREEN_HEIGHT);

            uniformgrid_clear(grid);
            uniformgrid_populate(grid, particles);
            uniformgrid_resolve_collisions(grid, particles);
            particle_constrain(particles, RESTITUTION, FRICTION, SCREEN_WIDTH, SCREEN_HEIGHT);

        }

        // uniformgrid_logger(grid);

        BeginDrawing();
            ClearBackground(BLACK);
            DrawFPS(5, 5);
            DrawText(TextFormat("Particle: %d", array_count(particles)),  5, 25, 20, LIME);

            particle_draw(particles);
        EndDrawing();
    }

    CloseWindow();
    uniformgrid_destroy(grid);
    array_destroy((void **) &particles);
    return 0;
} // gcc -o bin/main main.c src/particle/particle.c src/uniform_grid/uniform_grid.c -Iinclude -lraylib -lm -lladlib && ./bin/main