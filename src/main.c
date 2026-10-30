#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FPS 60
#define SUB_STEPS 12

#define PARTICLE_IMPLEMENTATION
#include "../include/particle.h"

#define REGULAR_GRID_IMPLEMENTATION
#include "../include/regular_grid.h"

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "spatial2d");
    SetTargetFPS(FPS);

    Particle_Array *particles_buffer = Particle_Array_Create(DEFAULT_CAPACITY);

    int grid_size   = 64;
    int grid_width  = (SCREEN_WIDTH  + grid_size - 1) / grid_size;
    int grid_height = (SCREEN_HEIGHT + grid_size - 1) / grid_size;

    size_t cells_size = (size_t) grid_width * grid_height;
    Cell *cells_buffer = Cell_Init(cells_size);

    if (!particles_buffer || !particles_buffer->items || !cells_buffer) {
        Particle_Array_destroy(particles_buffer);
        Cell_Destroy(cells_buffer, cells_size);
        CloseWindow();
        return 1;
    }

    float radius  = 25;
    float res     = 0.80;
    float fric    = 0.90;
    float air     = 0.;
    float gravity = 500.;
    Vector2 force = {.x = air, .y = gravity};

    float dt     = 1.0 / FPS;
    float sub_dt = dt / SUB_STEPS;
    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_SPACE)) { Particle_Array_MassSpawn(particles_buffer, 20, force,  radius, SCREEN_WIDTH, SCREEN_HEIGHT); }

        for (int i = 0; i < (int) SUB_STEPS; i++) {

            Particle_Update(particles_buffer->items, particles_buffer->count, sub_dt);
            Particle_Bound(particles_buffer->items, particles_buffer->count, 0.80, 0.80, SCREEN_WIDTH, SCREEN_HEIGHT);

            Cell_Clear(cells_buffer, cells_size);
            Cell_Populate(cells_buffer, particles_buffer->items, particles_buffer->count, SCREEN_WIDTH, SCREEN_HEIGHT, grid_size);
            Cell_Collision_Solver(particles_buffer, cells_buffer, SCREEN_WIDTH, SCREEN_HEIGHT, grid_size);

            Particle_Bound(particles_buffer->items, particles_buffer->count, res, fric, SCREEN_WIDTH, SCREEN_HEIGHT);

        }

        BeginDrawing();
            ClearBackground(BLACK);
            Particle_Draw(particles_buffer->items, particles_buffer->count);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}