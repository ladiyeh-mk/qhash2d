#include "../../include/uniform_grid/uniform_grid.h"
#include <ladlib/array.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

UniformGrid *uniformgrid_init(int screen_width, int screen_height, int cell_size) {
    if (!screen_width || !screen_height || !cell_size) { return NULL; }

    UniformGrid *grid = malloc(sizeof(UniformGrid));
    if (!grid) { return NULL; }

    grid->m_width      = screen_width;
    grid->m_height     = screen_height;
    grid->m_cols       = (int) ((screen_width + cell_size - 1) / cell_size);
    grid->m_rows       = (int) ((screen_height + cell_size - 1) / cell_size);

    grid->m_cell_size  = cell_size;
    grid->m_total_size = grid->m_cols * grid->m_rows;

    grid->m_ids        = array_new(sizeof(int *));
    if (!grid->m_ids) { free(grid); return NULL; }

    for (int i = 0; i < grid->m_total_size; i++) {

        int *temp = array_new(sizeof(int));
        if (!temp) {

            for (int k = 0; k < i; k++) {
                array_destroy((void **) &grid->m_ids[k]);
            }
            array_destroy((void **) &grid->m_ids);
            free(grid);

            return NULL;
        }

        if (!array_append((void **) &grid->m_ids, (void *) &temp, sizeof(int *))) {

            for (int k = 0; k < i; k++) {
                array_destroy((void **) &grid->m_ids[k]);
            }
            array_destroy((void **) &grid->m_ids);
            array_destroy((void **) &temp);
            free(grid);

            return NULL;
        }
    }

    return grid;
}

void uniformgrid_populate(UniformGrid *grid, Particle *particles) {
    if (!grid || !particles) { return; }

    int particles_count = array_count(particles);
    for (int i = 0; i < particles_count; i++) {

        int gx = (int) floorf(particles[i].m_current_pos.x / grid->m_cell_size); // gx & gy short for grid_x & grid_y
        int gy = (int) floorf(particles[i].m_current_pos.y / grid->m_cell_size); // but it's not like you didn't know it

        if (gx < 0) { gx = 0; }
        if (gx >= grid->m_cols) { gx = grid->m_cols - 1; }

        if (gy < 0) { gy = 0; }
        if (gy >= grid->m_rows) { gy = grid->m_rows - 1; }

        int grid_index = gx + grid->m_cols * gy;
        int *grid_cell = grid->m_ids[grid_index];

        array_append((void **) &grid_cell, (void *) &i, sizeof(int));
        grid->m_ids[grid_index] = grid_cell;

    }
}

void uniformgrid_clear(UniformGrid *grid) {
    if (!grid) { return; }

    for (int i = 0; i < grid->m_total_size; i++) {
        array_clear(grid->m_ids[i]);
    }
}

void uniformgrid_destroy(UniformGrid *grid) {
    if (!grid) { return; }

    for (int i = 0; i < grid->m_total_size; i++) {
        array_destroy((void **) &grid->m_ids[i]);
    }

    array_destroy((void **) &grid->m_ids);
    free(grid);
}

void uniformgrid_resolve_collisions(UniformGrid *grid, Particle *particles) {
    if (!grid || !particles) { return; }

    size_t particles_count = array_count(particles);
    for (size_t i = 0; i < particles_count; i++) {
        Particle *particle_a = &particles[i];

        int gx = (int) floorf(particle_a->m_current_pos.x / grid->m_cell_size); // gx & gy short for grid_x & grid_y
        int gy = (int) floorf(particle_a->m_current_pos.y / grid->m_cell_size); // but it's not like you didn't know it

        if (gx < 0) { gx = 0; }
        if (gx >= grid->m_cols) { gx = grid->m_cols - 1; }

        if (gy < 0) { gy = 0; }
        if (gy >= grid->m_rows) { gy = grid->m_rows - 1; }

        for (int col = gx - 1; col <= gx + 1; col++) {
            for (int row = gy - 1; row <= gy + 1; row++) {

                if (col < 0 || col >= grid->m_cols ||
                    row < 0 || row >= grid->m_rows) {

                    continue;
                }

                int grid_index = col + grid->m_cols * row;
                int *grid_cell = grid->m_ids[grid_index];

                int grid_cell_count = array_count(grid_cell);
                for (int j = 0; j < grid_cell_count; j++) {

                    int id = grid_cell[j];
                    if (id <= i) { continue; }

                    Particle *particle_b = &particles[id];

                    float dx = particle_b->m_current_pos.x - particle_a->m_current_pos.x;
                    float dy = particle_b->m_current_pos.y - particle_a->m_current_pos.y;

                    float dist_sq  = dx * dx + dy * dy;
                    float min_dist = particle_b->m_radius + particle_a->m_radius;
                    if (dist_sq > min_dist * min_dist) { continue; }

                    float dist = sqrtf(dist_sq);
                    if (dist < 0.0001f) { continue; }

                    float overlap = min_dist - dist;
                    float nx      = dx / dist;
                    float ny      = dy / dist;

                    particle_a->m_current_pos.x -= nx * overlap * 0.5f;
                    particle_a->m_current_pos.y -= ny * overlap * 0.5f;

                    particle_b->m_current_pos.x += nx * overlap * 0.5f;
                    particle_b->m_current_pos.y += ny * overlap * 0.5f;

                }

            }
        }
    }
}