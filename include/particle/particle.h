#ifndef __PARTICLE_H
#define __PARTICLE_H

#include <stdint.h>
#include <raylib.h>
#include <stdio.h>

typedef struct {

    Vector2 m_acc;
    Vector2 m_current_pos;
    Vector2 m_previous_pos;

    float m_radius;
    Color m_color;

} Particle;

void particle_draw(Particle *particles);
void particle_update(Particle *particles, float dt);
void particle_constrain(Particle *particles, float restitution, float friction, int screen_width, int screen_height);

Particle particle_create(Vector2 pos, Vector2 initial_vel, Vector2 force, float radius, Color color);
void particle_spawn_random(Particle **particles, size_t count, Vector2 force, float radius, int screen_width, int screen_height);

#endif