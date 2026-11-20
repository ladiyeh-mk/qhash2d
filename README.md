
# qhash2d
 
A real-time 2D particle simulation written in C using [raylib](https://www.raylib.com/).
 
The simulation uses **Verlet integration** with physics substepping for particle movement, and a **uniform grid spatial partitioning system** to efficiently detect and resolve particle collisions.
 
## Features
 
- **Verlet integration**: computes particle motion using current and previous positions, with acceleration applied over time.
- **Physics substepping**: divides each frame into smaller simulation steps for more stable collision handling.
- **Uniform grid spatial partitioning**: organizes particles into grid cells, reducing unnecessary collision checks by restricting detection to nearby cells.
- **Position-based collision resolution**: resolves overlapping particles by correcting their positions along the collision normal.
- **Wall constraints**: handles screen boundaries with configurable restitution and friction.
- **Random particle spawning**: generates particles at random positions with configurable initial velocity, acceleration, radius, and color.
- **Grid debug overlay**: optionally displays grid boundaries and the number of particles occupying each cell.
## Project structure
 
```
qhash2d/
├── main.c
├── include/
│   ├── particle/
│   │   └── particle.h
│   └── uniform_grid/
│       └── uniform_grid.h
├── src/
│   ├── particle/
│   │   └── particle.c
│   └── uniform_grid/
│       └── uniform_grid.c
└── bin/
```
 
## Dependencies
 
* [raylib](https://www.raylib.com/) — graphics and window management.
* [ladlib](https://github.com/ladiyeh-mk/ladlib) — custom C library providing generic data structures, including dynamic arrays.
## Build and run
 
Make sure raylib is installed. `ladlib` is a project dependency and is currently under development.
 
```bash
mkdir -p bin
 
gcc -o bin/main \
    main.c \
    src/particle/particle.c \
    src/uniform_grid/uniform_grid.c \
    -Iinclude \
    -lraylib -lm -lladlib
 
./bin/main
```
 
## Controls
 
| Key     | Action              |
| ------- | ------------------- |
| `SPACE` | Spawn 50 particles  |
| `C`     | Clear all particles |
 
## Configuration
 
The following constants in `main.c` control the simulation:
 
| Constant        | Description                  | Default |
| --------------- | ---------------------------- | ------- |
| `SCREEN_WIDTH`  | Window width in pixels       | `800`   |
| `SCREEN_HEIGHT` | Window height in pixels      | `600`   |
| `FPS`           | Target frame rate            | `60`    |
| `SUB_STEPS`     | Physics substeps per frame   | `12`    |
| `RADIUS`        | Particle radius              | `10`    |
| `CELL_SIZE`     | Uniform grid cell size       | `100`   |
| `FRICTION`      | Wall friction coefficient    | `0.1`   |
| `RESTITUTION`   | Wall restitution coefficient | `0.8`   |
 
Particles are spawned without acceleration by default. To introduce gravity, pass a downward acceleration vector to `particle_spawn_random`, for example:
 
```c
(Vector2) { 0, 1000 }
```
 
## Grid debug overlay
 
The uniform grid includes an optional debug logger that displays cell boundaries and particle counts.
 
Enable it by defining `UNIFORMGRID_LOGGER` before including the header:
 
```c
#define UNIFORMGRID_LOGGER
#include "include/uniform_grid/uniform_grid.h"
```
 
Then call the logger inside the drawing loop:
 
```c
BeginDrawing();
 
    ClearBackground(BLACK);
 
    particle_draw(particles);
    uniformgrid_glogger(grid);
 
EndDrawing();
```
 
## Simulation loop
 
Each frame is divided into multiple physics substeps. Each substep performs the following operations:
 
1. **Integrate particles** using Verlet integration:
```
next = 2 * current - previous + acc * dt^2
```
2. **Clear the uniform grid** to remove the previous substep's particle assignments.
3. **Populate the grid** by assigning particles to their corresponding cells.
4. **Resolve collisions** by checking particles in neighboring grid cells and correcting overlapping positions.
5. **Apply wall constraints** to keep particles inside the simulation boundaries, accounting for restitution and friction.
This approach avoids checking every particle against every other particle, making collision detection more efficient as the particle count increases.
 
## License
 
Released under the MIT License.
 
