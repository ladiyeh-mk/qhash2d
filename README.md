# qhash2d

A real-time 2D particle simulation written in C with [raylib](https://www.raylib.com/). It uses Verlet integration and substepping for stable motion, and a regular grid spatial partition so each particle only checks collisions against its nearby neighbors instead of every other particle.

## Features

- **Verlet integration**: velocity is implicit (`current_pos - previous_pos`), which keeps the integrator simple and stable.
- **Substepping**: each frame is split into several smaller steps to prevent tunneling and keep stacks stiff.
- **Regular grid broad-phase**: particles are bucketed into a uniform grid, and collisions are only tested against the 3x3 neighboring cells.
- **Position-based collision resolution**, weighted by inverse mass.
- **Wall collisions** with configurable restitution and friction.

## Project structure

```
src/main.c               simulation loop and setup
include/particle.h       particles, dynamic array, integration, wall bounds
include/regular_grid.h   grid cells, population, collision solver
```

Both headers are single-file style: define `PARTICLE_IMPLEMENTATION` and `REGULAR_GRID_IMPLEMENTATION` in exactly one source file before including them.

## Build and run

Requires raylib to be installed.

```bash
mkdir -p bin
gcc -o bin/main src/main.c -lraylib -lm && ./bin/main
```

## Controls

| Key     | Action                |
|---------|-----------------------|
| `SPACE` | Spawn 20 particles    |

## Tuning

Parameters in `main.c`:

| Name        | Meaning                                              |
|-------------|------------------------------------------------------|
| `SUB_STEPS` | Physics steps per frame (more = more stable)         |
| `grid_size` | Cell size in px, keep it >= 2 * particle radius      |
| `radius`    | Particle radius                                      |
| `gravity`   | Downward acceleration                                |
| `res`       | Wall restitution (bounciness)                        |
| `fric`      | Wall friction                                        |

## How it works

Each substep:

1. Integrate: `next = 2*current - previous + acc * dt^2`
2. Rebuild the grid from particle positions
3. For each particle, test the particles in the 3x3 neighboring cells
4. For overlapping pairs, push them apart along the contact normal, weighted by inverse mass
5. Clamp particles inside the window

## License

Released under the [MIT License](LICENSE).
