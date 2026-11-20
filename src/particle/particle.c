#include <stdio.h>
#include <raylib.h>
#include <stdint.h>
#include <ladlib/array.h>

#include "../../include/particle/particle.h"

void particle_draw(Particle *particles) {
    if (particles) {

        size_t particles_count = array_count(particles);
        for (size_t i = 0; i < particles_count; i++) {

            DrawCircleV(particles[i].m_current_pos, particles[i].m_radius, particles[i].m_color);

        }

    }
}

void particle_update(Particle *particles, float dt) {
    if (particles) {

        size_t particles_count = array_count(particles);
        for (size_t i = 0; i < particles_count; i++) {

            Vector2 vel = {
                .x = particles[i].m_current_pos.x - particles[i].m_previous_pos.x,
                .y = particles[i].m_current_pos.y - particles[i].m_previous_pos.y
            };
            particles[i].m_previous_pos = particles[i].m_current_pos;

            particles[i].m_current_pos.x += vel.x + particles[i].m_acc.x * dt * dt;
            particles[i].m_current_pos.y += vel.y + particles[i].m_acc.y * dt * dt;

        }

    }
}

void particle_constrain(Particle *particles, float restitution, float friction, int screen_width, int screen_height) {
    if (particles) {

        size_t particles_count = array_count(particles);
        for (size_t i = 0; i < particles_count; i++) {

            Vector2 vel = {
                .x = particles[i].m_current_pos.x - particles[i].m_previous_pos.x,
                .y = particles[i].m_current_pos.y - particles[i].m_previous_pos.y
            };

            if (particles[i].m_current_pos.x - particles[i].m_radius < 0) {

                particles[i].m_current_pos.x = particles[i].m_radius;
                particles[i].m_previous_pos  = (Vector2) {
                    .x = particles[i].m_current_pos.x + (vel.x * restitution),
                    .y = particles[i].m_current_pos.y - (vel.y * (1.f - friction))
                };

            }

            if (particles[i].m_current_pos.x + particles[i].m_radius > screen_width) {

                particles[i].m_current_pos.x = screen_width - particles[i].m_radius;
                particles[i].m_previous_pos  = (Vector2) {
                    .x = particles[i].m_current_pos.x + (vel.x * restitution),
                    .y = particles[i].m_current_pos.y - (vel.y * (1.f - friction))
                };

            }

            if (particles[i].m_current_pos.y - particles[i].m_radius < 0) {

                particles[i].m_current_pos.y = particles[i].m_radius;
                particles[i].m_previous_pos  = (Vector2) {
                    .y = particles[i].m_current_pos.y + (vel.y * restitution),
                    .x = particles[i].m_current_pos.x - (vel.x * (1.f - friction))
                };

            }

            if (particles[i].m_current_pos.y + particles[i].m_radius > screen_height) {

                particles[i].m_current_pos.y = screen_height - particles[i].m_radius;
                particles[i].m_previous_pos  = (Vector2) {
                    .y = particles[i].m_current_pos.y + (vel.y * restitution),
                    .x = particles[i].m_current_pos.x - (vel.x * (1.f - friction))
                };

            }

        }
    
    }
}

Particle particle_create(Vector2 pos, Vector2 initial_vel, Vector2 force, float radius, Color color) {
    return (Particle) {
        .m_acc          = force,
        .m_current_pos  = pos,
        .m_previous_pos = (Vector2) {
            .x = pos.x - initial_vel.x,
            .y = pos.y - initial_vel.y
        },

        .m_radius = radius,
        .m_color  = color
    };
}

void particle_spawn_random(Particle **particles, size_t count, Vector2 force, float radius, int screen_width, int screen_height) {
    if (!particles || !*particles) { return; }

    for (size_t i = 0; i < count; i++) {

        Vector2 pos = {
            .x = GetRandomValue(radius, screen_width - radius),
            .y = GetRandomValue(radius, screen_height - radius)
        };

        Vector2 initial_vel = {
            .x = GetRandomValue(-100, 100),
            .y = GetRandomValue(-100, 100)
        };

        float t = (float) i / count;
        Color color = ColorFromHSV(t * 360.f, 1.f, 1.f);

        Particle particle = particle_create(pos, initial_vel, force, radius, color);
        if (!array_append((void **) particles, (void *) &particle, sizeof(Particle))) {
            printf("[WARNING]: Couldn't append for some reasons in func: %s()", __func__);
        }

    }
}