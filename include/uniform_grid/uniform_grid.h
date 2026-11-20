#ifndef __UNIFORM_GRID_H
#define __UNIFORM_GRID_H

#include <ladlib/array.h>
#include "../particle/particle.h"

typedef struct {

    int m_rows, m_cols;
    int m_width, m_height;

    int **m_ids;
    int m_cell_size;
    int m_total_size;

} UniformGrid;

UniformGrid *uniformgrid_init(int screen_width, int screen_height, int cell_size);
void uniformgrid_populate(UniformGrid *grid, Particle *particles);
void uniformgrid_clear(UniformGrid *grid);
void uniformgrid_destroy(UniformGrid *grid);
void uniformgrid_resolve_collisions(UniformGrid *grid, Particle *particles);

#ifdef UNIFORMGRID_LOGGER
#include <stdio.h>

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

#endif

#endif