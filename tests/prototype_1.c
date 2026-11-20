#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 1800
#define SCREEN_HEIGHT 1000
#define FPS 60
#define SUB_STEPS 1

#define CELL_SIZE 75
#define CELL_WIDTH (SCREEN_WIDTH / CELL_SIZE)
#define CELL_HEIGHT (SCREEN_HEIGHT / CELL_SIZE)

// ================================
// ===         POINT            ===
// ================================

typedef struct {
    Vector2 current_pos, previous_pos;
    float radius;
} Particle;

void Draw_Particles(Particle *particles, size_t particles_count) {
    for (size_t i = 0; i < particles_count; i++) {
        DrawCircleV(particles[i].current_pos, particles[i].radius, LIME);
    }
}

void Update_Particles(Particle *particles, size_t particles_count, Vector2 gravity, float dt) {
    for (size_t i = 0; i < particles_count; i++) {
        Particle p = particles[i];

        Vector2 vel = {0};
        vel.x = p.current_pos.x - p.previous_pos.x;
        vel.y = p.current_pos.y - p.previous_pos.y;

        Vector2 acc_step = {0};
        acc_step.x = gravity.x * dt * dt;
        acc_step.y = gravity.y * dt * dt;

        p.previous_pos = p.current_pos;
        p.current_pos.x += vel.x + acc_step.x;
        p.current_pos.y += vel.y + acc_step.y;

        particles[i] = p;
    }
}

void Constrain_Particles(Particle *particles, size_t particles_count, float restitution, float friction) {
    for (size_t i = 0; i < particles_count; i++) {
        Particle p = particles[i];
        Vector2 vel = {0};
        vel.x = p.current_pos.x - p.previous_pos.x;
        vel.y = p.current_pos.y - p.previous_pos.y;

        // Left wall
        if (p.current_pos.x - p.radius < 0) {
            p.current_pos.x = p.radius;
            // Reflect X (normal) with restitution, dampen Y (tangent) with friction
            p.previous_pos.x = p.current_pos.x + (vel.x * restitution);
            p.previous_pos.y = p.current_pos.y - (vel.y * (1.0f - friction));
        }

        // Right wall
        if (p.current_pos.x + p.radius > SCREEN_WIDTH) {
            p.current_pos.x = SCREEN_WIDTH - p.radius;
            // Reflect X (normal) with restitution, dampen Y (tangent) with friction
            p.previous_pos.x = p.current_pos.x + (vel.x * restitution);
            p.previous_pos.y = p.current_pos.y - (vel.y * (1.0f - friction));
        }

        // Top wall
        if (p.current_pos.y - p.radius < 0) {
            p.current_pos.y = p.radius;
            // Reflect Y (normal) with restitution, dampen X (tangent) with friction
            p.previous_pos.y = p.current_pos.y + (vel.y * restitution);
            p.previous_pos.x = p.current_pos.x - (vel.x * (1.0f - friction));
        }

        // Bottom wall
        if (p.current_pos.y + p.radius > SCREEN_HEIGHT) {
            p.current_pos.y = SCREEN_HEIGHT - p.radius;
            // Reflect Y (normal) with restitution, dampen X (tangent) with friction
            p.previous_pos.y = p.current_pos.y + (vel.y * restitution);
            p.previous_pos.x = p.current_pos.x - (vel.x * (1.0f - friction));
        }
        particles[i] = p;
    }
}

// ================================
// ===      PARTICLEARRAY       ===
// ================================

typedef struct {
    Particle *items;
    size_t count, capacity;
} ParticleArray;

ParticleArray ParticleArray_Create(size_t capacity) {
    if (!capacity) return (ParticleArray) {0};

    ParticleArray arr = {0};
    arr.capacity = capacity;
    arr.items = malloc(sizeof(Particle) * capacity);
    if (!arr.items) return (ParticleArray) {0};

    return arr;
}

void ParticleArray_Push(ParticleArray *arr, const Particle p) {
    if (arr->count >= arr->capacity) {
        size_t new_capacity = (arr->capacity) ? arr->capacity * 2. : 16;
        Particle *data = arr->items;

        data = realloc(data, sizeof(Particle) * new_capacity);
        if (!data) return;

        arr->items = data;
        arr->capacity = new_capacity;
    }
    arr->items[arr->count++] = p;
}

void ParticleArray_MassSpawn(ParticleArray *arr, int particles_count) {
    float radius = 25.0f;
    float min_x = radius;
    float max_x = (float)SCREEN_WIDTH - radius;
    float min_y = radius;
    float max_y = (float)SCREEN_HEIGHT - radius;

    for (int i = 0; i < particles_count; i++) {
        // Uniform float generation between [min, max]
        float rx = min_x + ((float)rand() / (float)RAND_MAX) * (max_x - min_x);
        float ry = min_y + ((float)rand() / (float)RAND_MAX) * (max_y - min_y);

        Particle p = {
            .current_pos = { .x = rx, .y = ry },
            .previous_pos = { .x = rx, .y = ry }, // Zero initial velocity
            .radius = radius
        };
        
        ParticleArray_Push(arr, p);
    }
}

void ParticleArray_Free(ParticleArray *arr) {
    free(arr->items);

    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

// ================================
// ===         CELL             ===
// ================================

typedef struct {
    int *ids;
    size_t count, capacity;
} Cell;

Cell Cell_Create(size_t capacity) {
    if (!capacity) return (Cell) {0};

    Cell cell = {0};
    cell.capacity = capacity;
    cell.ids = malloc(sizeof(int) * capacity);
    if (!cell.ids) return (Cell) {0};

    return cell;
}

void Cell_Push(Cell *cell, int index) {
    if (cell->count >= cell->capacity) {
        size_t new_capacity = (cell->capacity) ? cell->capacity * 2 : 16;
        int *data = cell->ids;

        data = realloc(data, sizeof(int) * new_capacity);
        if (!data) return;

        cell->capacity = new_capacity;
        cell->ids = data;
    }
    cell->ids[cell->count++] = index;
}

void Cell_Clear(Cell *cells, size_t cells_count) {
    for (size_t i = 0; i < cells_count; i++) cells[i].count = 0;
}

void Cell_Populate(Cell *cells, ParticleArray *arr) {
    for (size_t i = 0; i < arr->count; i++) {
        Particle p = arr->items[i];

        size_t x = (size_t) (p.current_pos.x / CELL_SIZE);
        size_t y = (size_t) (p.current_pos.y / CELL_SIZE);

        
        if (x >= CELL_WIDTH) x = CELL_WIDTH - 1;
        if (y >= CELL_HEIGHT) y = CELL_HEIGHT - 1;

        size_t index = y * CELL_WIDTH + x;
        Cell *cell = &cells[index];

        Cell_Push(cell, i);
    }
}

void Cells_Free(Cell *cells, size_t cells_count) {
    for (size_t i = 0; i < cells_count; i++) {
        free(cells[i].ids);
    }
    free(cells);
}

// ================================
// ===     COLLISION SOLVER     ===
// ================================

int Check_Collision(const Particle *p1, const Particle *p2) {
    float dx = p2->current_pos.x - p1->current_pos.x;
    float dy = p2->current_pos.y - p1->current_pos.y;
    float distance_sq = (dx * dx) + (dy * dy);
    float min_dist = p1->radius + p2->radius;

    return distance_sq < (min_dist * min_dist);
}

void Resolve_Collision(Cell *cells, ParticleArray *arr) {
    for (size_t i = 0; i < arr->count; i++) {
        Particle *p1 = &arr->items[i];

        // Use signed integers and floorf to safely handle negative coordinates
        int cell_x = (int)floorf(p1->current_pos.x / CELL_SIZE);
        int cell_y = (int)floorf(p1->current_pos.y / CELL_SIZE);

        cell_x = (cell_x < 0) ? 0 : cell_x;
        cell_x = (cell_x >= CELL_WIDTH) ? CELL_WIDTH - 1 : cell_x;
        cell_y = (cell_y < 0) ? 0 : cell_y;
        cell_y = (cell_y >= CELL_HEIGHT) ? CELL_HEIGHT - 1 : cell_y;

        for (int col = cell_x - 1; col <= cell_x + 1; col++) {
            for (int row = cell_y - 1; row <= cell_y + 1; row++) {
                if (col < 0 || col >= (int)CELL_WIDTH ||
                    row < 0 || row >= (int)CELL_HEIGHT) {
                    continue;
                }

                size_t index = (size_t)row * CELL_WIDTH + (size_t)col;
                Cell *cell = &cells[index];

                for (size_t j = 0; j < cell->count; j++) {
                    int other_id = cell->ids[j];

                    if (other_id <= (int)i)
                        continue;

                    Particle *p2 = &arr->items[other_id];

                    if (!Check_Collision(p1, p2))
                        continue;

                    float dx = p2->current_pos.x - p1->current_pos.x;
                    float dy = p2->current_pos.y - p1->current_pos.y;
                    float distance = sqrtf(dx * dx + dy * dy);

                    if (distance < 0.0001f)
                        continue;

                    float overlap = (p1->radius + p2->radius) - distance;

                    float nx = dx / distance;
                    float ny = dy / distance;

                    float correction_x = nx * overlap * 0.5f;
                    float correction_y = ny * overlap * 0.5f;

                    // Positional response (leaves previous_pos unchanged to impart bounce velocity)
                    p1->current_pos.x -= correction_x;
                    p1->current_pos.y -= correction_y;

                    p2->current_pos.x += correction_x;
                    p2->current_pos.y += correction_y;
                }
            }
        }
    }
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Particle Engine test 1");
    SetTargetFPS(FPS);

    size_t cell_cap = 32;
    size_t n_cells = CELL_WIDTH * CELL_HEIGHT;
    Cell *cells = malloc(sizeof(Cell) * n_cells);
    for (size_t i = 0; i < n_cells; i++) cells[i] = Cell_Create(cell_cap);

    size_t n_particles = 1024;
    ParticleArray pa_array = ParticleArray_Create(n_particles);

    ParticleArray_MassSpawn(&pa_array, 200);

    Particle *particles = pa_array.items;
    size_t particles_count = pa_array.count;

    while(!WindowShouldClose()) {
        // updates aa vars ...;

        if (IsKeyPressed(KEY_P)) {
            ParticleArray_MassSpawn(&pa_array, 250);
            particles = pa_array.items;
            particles_count = pa_array.count;
        }

        float delta_time = (float) (1. / FPS);
        float sub_delta_time = (float) (delta_time / SUB_STEPS);
        for (int step = 0; step < SUB_STEPS; step++) {
            Update_Particles(particles, particles_count, (Vector2) {0., 2000.}, sub_delta_time);

            Cell_Clear(cells, n_cells);
            Cell_Populate(cells, &pa_array);

            Resolve_Collision(cells, &pa_array);
            Constrain_Particles(particles, particles_count, 0.8, 0.1);
        }

        // Drawing ...;
        BeginDrawing();
            ClearBackground(BLACK);
            DrawFPS(SCREEN_WIDTH - 90, 20);
            DrawText(TextFormat("Particles: %ld", particles_count), 5, 10, 20, YELLOW);

            Draw_Particles(particles, particles_count);
        
        EndDrawing();
    }

    CloseWindow();

    Cells_Free(cells, n_cells);
    ParticleArray_Free(&pa_array);
} // gcc -o bin/prototype_1 tests/prototype_1.c -lraylib -lm && bin/prototype_1 