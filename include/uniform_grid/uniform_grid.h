#ifndef __UNIFORM_GRID_H
#define __UNIFORM_GRID_H

#include <raylib.h>
#include <ladlib/array.h>
#include "../particle/particle.h"


typedef struct {

    int **ids;
    int cell_size;
    int rows, cols;
    int width, height;
    int total_size;

} UniformGrid;

UniformGrid *uniformgrid_init(int screen_width, int screen_height, int cell_size);
void uniformgrid_populate(UniformGrid *grid, Particle *particles);
void uniformgrid_clear(UniformGrid *grid);
void uniformgrid_destroy(UniformGrid *grid);
void uniformgrid_resolve_collisions(UniformGrid *grid, Particle *particles);

#ifdef UNIFORMGRID_LOGGER
#include <stdio.h>

void uniformgrid_logger(UniformGrid *grid) {
    if (!grid || !grid->ids) { return; }

}

#endif

#endif