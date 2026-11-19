#include "../../include/uniform_grid/uniform_grid.h"
#include <math.h>
#include <stdlib.h>
#include <ladlib/array.h>

UniformGrid *uniformgrid_init(int screen_width, int screen_height, int cell_size) {
    if (!screen_width || !screen_height || !cell_size) { return NULL; }

    UniformGrid *grid = malloc(sizeof(UniformGrid));
    if (!grid) { return NULL; }

    grid->width     = screen_width;
    grid->height    = screen_height;
    grid->cell_size = cell_size;
    
    grid->cols       = (screen_width  + cell_size - 1) / cell_size;
    grid->rows       = (screen_height + cell_size - 1) / cell_size;
    grid->total_size = grid->cols * grid->rows;

    grid->ids = array_new(sizeof(int *));
    if (!grid->ids) { free(grid); return NULL; }

    for (int i = 0; i < grid->total_size; i++) {
        int *temp = array_new(sizeof(int));
        if (!temp) { 
            
            for (int j = 0; j < i; j++) { 
                array_destroy((void **) &grid->ids[j]); 
            } 
            array_destroy((void **) &grid->ids); 
            free(grid); 
            return NULL; 
        }
        
        // Append the new cell array to the outer container
        array_append((void **) &grid->ids, (void *) &temp, sizeof(int *));
    }

    return grid;
}

void uniformgrid_populate(UniformGrid *grid, Particle *particles) {
    if (!grid || !grid->ids || !particles) { return; }

    int count = (int) array_count(particles);
    for (int i = 0; i < count; i++) {
        int gx = (int) (particles[i].m_current_pos.x / grid->cell_size);
        int gy = (int) (particles[i].m_current_pos.y / grid->cell_size);

        if (gx < 0) { gx = 0; }
        if (gy < 0) { gy = 0; }
        if (gx >= grid->cols) { gx = grid->cols - 1; }
        if (gy >= grid->rows) { gy = grid->rows - 1; }

        int cell_index = gx + grid->cols * gy;
        
        // Safely extract, append, and re-assign the inner cell array pointer ...
        int *cell_array = grid->ids[cell_index];
        array_append((void **) &cell_array, (void *) &i, sizeof(int));
        grid->ids[cell_index] = cell_array;
    }
}

void uniformgrid_clear(UniformGrid *grid) {
    if (!grid || !grid->ids) { return; }

    int total = (int) array_count(grid->ids);
    for (int i = 0; i < total; i++) {
        if (grid->ids[i]) { array_clear(grid->ids[i]); }
    }
}

void uniformgrid_destroy(UniformGrid *grid) {
    if (!grid) return;
    if (grid->ids) {
        int total = (int) array_count(grid->ids);
        for (int i = 0; i < total; i++) {
            if (grid->ids[i]) {
                array_destroy((void **) &grid->ids[i]);
            }
        }
        array_destroy((void **) &grid->ids);
    }
    free(grid);
}

void uniformgrid_resolve_collisions(UniformGrid *grid, Particle *particles) {
    if (!grid || !particles) { return; }

    int count = array_count(particles);
    for (int i = 0; i < count; i++) {
        Particle *particle_a = &particles[i];

        int gx = (int) (particles[i].m_current_pos.x / grid->cell_size);
        int gy = (int) (particles[i].m_current_pos.y / grid->cell_size);

        if (gx < 0) { gx = 0; }
        if (gy < 0) { gy = 0; }
        if (gx >= grid->cols) { gx = grid->cols - 1; }
        if (gy >= grid->rows) { gy = grid->rows - 1; }

        for (int c = -1; c <= 1; c++) {
            for (int r = -1; r <= 1; r++) {
                int cx = gx + c;
                int cy = gy + r;
                
                if (cx < 0 || cx >= grid->cols || cy < 0 || cy >= grid->rows) { continue; }
                int *cell_array      = grid->ids[cx + grid->cols * cy];
                int cell_array_count = array_count(cell_array);

                for (int j = 0; j < cell_array_count; j++) {
                    int id = cell_array[j];
                    if (i >= id) { continue; }
                    Particle *particle_b = &particles[id];

                    float dx = particle_b->m_current_pos.x - particle_a->m_current_pos.x;
                    float dy = particle_b->m_current_pos.y - particle_a->m_current_pos.y;

                    float dist_sq  = dx * dx + dy * dy;
                    float min_dist = particle_b->m_radius + particle_a->m_radius;
                    if (dist_sq >= min_dist * min_dist) { continue; }

                    float dist = sqrtf(dist_sq);
                    float nx, ny;
                    
                    if (dist < 1e-6f) { nx = 1.; ny = 0.; }
                    else              { nx = dx / dist; ny = dy / dist; }

                    float overlap = min_dist - dist;
                    float correction = overlap * 0.5f;

                    // Push particle A away from B
                    particle_a->m_current_pos.x -= nx * correction;
                    particle_a->m_current_pos.y -= ny * correction;

                    // Push particle B away from A
                    particle_b->m_current_pos.x += nx * correction;
                    particle_b->m_current_pos.y += ny * correction;
                }
            }
        }
    }
}