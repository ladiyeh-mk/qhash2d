#include "../../include/particle/particle.h"
#include <math.h>
#include <raylib.h>

void particle_draw(Particle *particles) {
    if (!particles) { return; }

    int count = array_count(particles);
    for (int i = 0; i < count; i++) {
        DrawCircleV(particles[i].m_current_pos, particles[i].m_radius, particles[i].m_color);
    }
}

void particle_update(Particle *particles, float dt) {
    if (!particles) { return; }
    int count = array_count(particles);

    for (int i = 0; i < count; i++) {
        Vector2 temp     = particles[i].m_current_pos;
        Vector2 velocity = {
            .x = particles[i].m_current_pos.x - particles[i].m_previous_pos.x,
            .y = particles[i].m_current_pos.y - particles[i].m_previous_pos.y
        };

        //      next_pos            (2 * current_pos) + current_pos - previous_pos + (acc * dt ** 2)
        //          |                                   |                                   |
        particles[i].m_current_pos.x = particles[i].m_current_pos.x + velocity.x + (particles[i].m_acc.x * dt * dt);
        particles[i].m_current_pos.y = particles[i].m_current_pos.y + velocity.y + (particles[i].m_acc.y * dt * dt);

        particles[i].m_previous_pos = temp;
    }
}

void particle_constrain(Particle *particles, float restitution, float friction, int screen_width, int screen_height) {
    if (!particles) { return; }
    int count = array_count(particles);

    for (int i = 0; i < count; i++) {
        Particle *current_particle = &particles[i];
        Vector2 vel = {0};

        //  === LEFT & RIGHT WALL's ===
        if (current_particle->m_current_pos.x - current_particle->m_radius < 0) {
            vel = (Vector2) {
                .x = current_particle->m_current_pos.x - current_particle->m_previous_pos.x,
                .y = current_particle->m_current_pos.y - current_particle->m_previous_pos.y
            };

            current_particle->m_current_pos.x = current_particle->m_radius;
            current_particle->m_previous_pos  = (Vector2) {
                .x = current_particle->m_current_pos.x + vel.x * restitution,
                .y = current_particle->m_current_pos.y - vel.y * (1 - friction)
            };
        }

        if (current_particle->m_current_pos.x + current_particle->m_radius > screen_width) {
            vel = (Vector2) {
                .x = current_particle->m_current_pos.x - current_particle->m_previous_pos.x,
                .y = current_particle->m_current_pos.y - current_particle->m_previous_pos.y
            };

            current_particle->m_current_pos.x = screen_width - current_particle->m_radius;
            current_particle->m_previous_pos  = (Vector2) {
                .x = current_particle->m_current_pos.x + vel.x * restitution,
                .y = current_particle->m_current_pos.y - vel.y * (1 - friction)
            };
        }

        //  === UPPER & BOTTOM WALL's ===
        if (current_particle->m_current_pos.y - current_particle->m_radius < 0) {
            vel = (Vector2) {
                .x = current_particle->m_current_pos.x - current_particle->m_previous_pos.x,
                .y = current_particle->m_current_pos.y - current_particle->m_previous_pos.y
            };

            current_particle->m_current_pos.y = current_particle->m_radius;
            current_particle->m_previous_pos  = (Vector2) {
                .x = current_particle->m_current_pos.x - vel.x * (1 - friction),
                .y = current_particle->m_current_pos.y + vel.y * restitution
            };
        }

        if (current_particle->m_current_pos.y + current_particle->m_radius > screen_height) {
            vel = (Vector2) {
                .x = current_particle->m_current_pos.x - current_particle->m_previous_pos.x,
                .y = current_particle->m_current_pos.y - current_particle->m_previous_pos.y
            };

            current_particle->m_current_pos.y = screen_height - current_particle->m_radius;
            current_particle->m_previous_pos  = (Vector2) {
                .x = current_particle->m_current_pos.x - vel.x * (1 - friction),
                .y = current_particle->m_current_pos.y + vel.y * restitution
            };
        }
    }
}

static inline Color particle_color() {
    return (Color) {
        .r = GetRandomValue(0, 255),
        .g = GetRandomValue(0, 255),
        .b = GetRandomValue(0, 255),
        .a = 255,
    };
}

Particle particle_create(Vector2 pos, Vector2 initial_vel, Vector2 force, float radius) {
    return (Particle) {
        .m_acc          = force,
        .m_current_pos  = pos,
        .m_previous_pos = (Vector2) {
            .x = pos.x - initial_vel.x,
            .y = pos.y - initial_vel.y
        },
        .m_radius       = radius,
        .m_color        = particle_color(),
    };
}

void particle_spawn_random(Particle **particles, int count, float radius, Vector2 force, int screen_width, int screen_height) {
    int r = (int)radius;
    for (int i = 0; i < count; i++) {
        float x = (float)GetRandomValue(r, screen_width - r);
        float y = (float)GetRandomValue(r, screen_height - r);

        // Small random initial velocity vector
        Vector2 initial_vel = {
            .x = (float)GetRandomValue(-100, 100) / 20.f,
            .y = (float)GetRandomValue(-100, 100) / 20.f
        };

        Particle p = particle_create((Vector2) {x, y}, initial_vel, force, radius);
        array_append((void **) particles, (void *) &p, sizeof(Particle));
    }
}

void particle_spawn_burst(Particle **particles, Vector2 center, int count, float radius, Vector2 force) {
    const float golden_angle = 2.39996323f;   // sunflower spiral spacing

    for (int i = 0; i < count; i++) {
        float a = i * golden_angle;
        float d = radius * 2.0f * sqrtf(i + 0.5f);   // spacing ~ one diameter

        Vector2 dir = { cosf(a), sinf(a) };

        Vector2 pos = {
            .x = center.x + dir.x * d,
            .y = center.y + dir.y * d
        };

        float speed = (float)GetRandomValue(5, 20) / 10.f;

        Vector2 initial_vel = { dir.x * speed, dir.y * speed };

        Particle p = particle_create(pos, initial_vel, force, radius);
        array_append((void **) particles, (void *) &p, sizeof(Particle));
    }
}