#ifndef REGULAR_GRID_
#define REGULAR_GRID_

#include <math.h>
#include <stdlib.h>

#include "raylib.h"
#include "particle.h"

#define DEFAULT_CELL_CAPACITY 32

typedef struct {

    int *ids;
    size_t count, capacity;

} Cell;

Cell Cell_Create(size_t capacity);
Cell *Cell_Init(size_t capacity);
void Cell_Push(Cell *cell, int id);
void Cell_Clear(Cell *cells, size_t count);
void Cell_Populate(Cell *cells, Particle *particles, size_t count, int screen_width, int screen_height, int cell_size);
void Cell_Destroy(Cell *cells, size_t count);
void Cell_Collision_Solver(Particle_Array *array, Cell *cells, int screen_width, int screen_height, int cell_size);

#ifdef REGULAR_GRID_IMPLEMENTATION

Cell Cell_Create(size_t capacity) {
    if (!capacity) capacity = DEFAULT_CELL_CAPACITY;
    Cell cell = {0};

    cell.count    = 0;
    cell.capacity = capacity;
    cell.ids      = malloc(sizeof(int) * capacity);

    if (!cell.ids) return (Cell) {0};
    return cell;
}

Cell *Cell_Init(size_t capacity) {
    if (!capacity) return NULL;

    Cell *cells = malloc(sizeof(Cell) * capacity);
    if (!cells) return NULL;

    for (int i = 0; i < (int) capacity; i++) { cells[i] = Cell_Create(DEFAULT_CELL_CAPACITY); }
    return cells;
}

void Cell_Push(Cell *cell, int id) {
    if (cell) {
        if (cell->count == cell->capacity) {
            size_t new_cap = (cell->capacity) ? cell->capacity * 2 : DEFAULT_CELL_CAPACITY;
            int *temp      = realloc(cell->ids, sizeof(int) * new_cap);
            if (!temp) return;

            cell->ids      = temp;
            cell->capacity = new_cap;
        }

        cell->ids[cell->count++] = id;
    }
}

void Cell_Clear(Cell *cells, size_t count) {
    if (!cells) return;
    for (size_t i = 0; i < count; i++) cells[i].count = 0;
}

void Cell_Populate(Cell *cells, Particle *particles, size_t count, int screen_width, int screen_height, int cell_size) {
    if (!cells || !particles) return;

    int grid_width  = (screen_width  + cell_size - 1) / cell_size;
    int grid_height = (screen_height + cell_size - 1) / cell_size;

    for (size_t i = 0; i < count; i++) {
        int grid_x = (int) (particles[i].current_pos.x / cell_size);
        int grid_y = (int) (particles[i].current_pos.y / cell_size);

        if (grid_x < 0) grid_x = 0;
        if (grid_y < 0) grid_y = 0;
        if (grid_x >= grid_width)  grid_x = grid_width  - 1;
        if (grid_y >= grid_height) grid_y = grid_height - 1;

        Cell_Push(&cells[grid_x + grid_width * grid_y], (int) i);
    }
}

void Cell_Destroy(Cell *cells, size_t count) {
    if (!cells) return;
    for (size_t i = 0; i < count; i++) free(cells[i].ids);
    free(cells);
}

void Cell_Collision_Solver(Particle_Array *array, Cell *cells, int screen_width, int screen_height, int cell_size) {
    if (!array || !array->items || !cells) return;

    int grid_width  = (screen_width  + cell_size - 1) / cell_size;
    int grid_height = (screen_height + cell_size - 1) / cell_size;

    for (int i = 0; i < (int) array->count; i++) {
        Particle *a = &array->items[i];

        int grid_x = (int) (a->current_pos.x / cell_size);
        int grid_y = (int) (a->current_pos.y / cell_size);

        if (grid_x < 0) grid_x = 0;
        if (grid_y < 0) grid_y = 0;
        if (grid_x >= grid_width)  grid_x = grid_width  - 1;
        if (grid_y >= grid_height) grid_y = grid_height - 1;

        for (int oy = -1; oy <= 1; oy++) {
            for (int ox = -1; ox <= 1; ox++) {
                int cx = grid_x + ox;
                int cy = grid_y + oy;
                if (cx < 0 || cx >= grid_width || cy < 0 || cy >= grid_height) continue;

                Cell *cell = &cells[cx + grid_width * cy];

                for (size_t j = 0; j < cell->count; j++) {
                    int id = cell->ids[j];
                    if (i >= id) continue;
                    Particle *b = &array->items[id];

                    float dx = b->current_pos.x - a->current_pos.x;
                    float dy = b->current_pos.y - a->current_pos.y;

                    float dist_sq  = dx * dx + dy * dy;
                    float min_dist = a->radius + b->radius;
                    if (dist_sq >= min_dist * min_dist) continue;

                    float dist = sqrtf(dist_sq);
                    float nx, ny;
                    if (dist < 1e-6f) { nx = 1.f; ny = 0.f; }
                    else              { nx = dx / dist; ny = dy / dist; }

                    float overlap = min_dist - dist;
                    float w       = a->inv_mass + b->inv_mass;
                    if (w <= 0.f) continue;

                    float ka = overlap * (a->inv_mass / w);
                    float kb = overlap * (b->inv_mass / w);

                    a->current_pos.x -= nx * ka;
                    a->current_pos.y -= ny * ka;
                    b->current_pos.x += nx * kb;
                    b->current_pos.y += ny * kb;
                }
            }
        }
    }
}

#endif


#endif