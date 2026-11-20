#include <stdio.h>
#include <raylib.h>
#include <ladlib/array.h>

#include "../include/particle/particle.h"
#include "../include/uniform_grid/uniform_grid.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define FPS 60
#define SUB_STEPS 12

#define DT (1.f / FPS)
#define SUB_DT (DT / SUB_STEPS)

#define RADIUS 10
#define CELL_SIZE (RADIUS * 10)

#define FRICTION 0.1f
#define RESTITUTION 0.8f

void uniformgrid_glogger(UniformGrid *grid) {
    if (!grid || grid->m_cell_size <= 0) { return; }

    for (int x = 0; x < grid->m_cols; x++) {
        for (int y = 0; y < grid->m_rows; y++) {

            size_t grid_index      = x + grid->m_cols * y;
            int *grid_cell         = grid->m_ids[grid_index];
            size_t grid_cell_count = array_count(grid_cell);

            const char *label = TextFormat("%zu", grid_cell_count);

            int font_size = grid->m_cell_size / 3;
            if (font_size < 10) { font_size = 10; }
            if (font_size > 24) { font_size = 24; }

            // Shrink if still wider than the cell (with a little padding)
            while (font_size > 8 && MeasureText(label, font_size) > grid->m_cell_size - 8) {
                font_size--;
            }

            DrawText(label, x * grid->m_cell_size + 4, y * grid->m_cell_size + 4, font_size, YELLOW);

            DrawRectangleLines(x * grid->m_cell_size, y * grid->m_cell_size, grid->m_cell_size, grid->m_cell_size, YELLOW);

        }
    }
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "qhash2d");
    SetTargetFPS(FPS);

    Particle *particles = array_new(sizeof(Particle));
    if (!particles) {
        fprintf(stderr, "[ERROR]: Couldn't init particle array !\n");
        CloseWindow(); return 1;
    }

    UniformGrid *grid = uniformgrid_init(SCREEN_WIDTH, SCREEN_HEIGHT, CELL_SIZE);
    if (!grid) {
        fprintf(stderr, "[ERROR]: Couldn't init grid !\n");
        CloseWindow(); array_destroy((void **) &particles); return 1;
    }

    particle_spawn_random(&particles, 50, (Vector2) {0}, RADIUS, SCREEN_WIDTH, SCREEN_HEIGHT);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_C)) { array_clear(particles); }
        if (IsKeyPressed(KEY_SPACE)) { particle_spawn_random(&particles, 50, (Vector2) {0}, RADIUS, SCREEN_WIDTH, SCREEN_HEIGHT); }

        for (size_t i = 0; i < SUB_STEPS; i++) {

            particle_update(particles, SUB_DT);

            uniformgrid_clear(grid);
            uniformgrid_populate(grid, particles);
            uniformgrid_resolve_collisions(grid, particles, RESTITUTION, 0);

            particle_constrain(particles, RESTITUTION, FRICTION, SCREEN_WIDTH, SCREEN_HEIGHT);

        }
        

        BeginDrawing();
            ClearBackground(BLACK);
            DrawFPS(SCREEN_WIDTH - 80, 5);
            DrawText(TextFormat("Particles: %zu", array_count(particles)), 5, 5, 20, LIME);

            particle_draw(particles);
            uniformgrid_glogger(grid);
        EndDrawing();
    }
}

// run: gcc -o bin/uniform_grid_test tests/uniform_grid_test.c src/particle/particle.c src/uniform_grid/uniform_grid.c -Iinclude -lraylib -lm -lladlib && ./bin/uniform_grid_test