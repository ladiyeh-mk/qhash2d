#ifndef PARTICLE_H
#define PARTICLE_H

#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"

// =============================================
// ==========       PARTICLE     ===============
// =============================================

typedef struct {

    Vector2 acc;
    Vector2 current_pos;
    Vector2 previous_pos;

    float radius, inv_mass;

} Particle;

void Particle_Draw(Particle *particles, size_t count);
void Particle_Update(Particle *particles, size_t count, float dt);
void Particle_Bound(Particle *particles, size_t count, float res, float fric, int screen_width, int screen_height);

// =============================================
// ==========    PARTICLE ARRAY  ===============
// =============================================

#define DEFAULT_CAPACITY 32

typedef struct {

    Particle *items;
    size_t count, capacity;

} Particle_Array;

Particle_Array *Particle_Array_Create(size_t capacity);
void Particle_Array_Push(Particle_Array *array, Particle particle);
void Particle_Array_destroy(Particle_Array *array);
void Particle_Array_MassSpawn(Particle_Array *array, size_t n_particles, Vector2 force, float radius, int screen_width, int screen_height);

#ifdef PARTICLE_IMPLEMENTATION

// =============================================
// ==========       PARTICLE     ===============
// =============================================

void Particle_Draw(Particle *particles, size_t count) { for (int i = 0; i < (int) count; i++) DrawCircleV(particles[i].current_pos, particles[i].radius, WHITE); }
void Particle_Update(Particle *particles, size_t count, float dt) {
    for (int i = 0; i < (int) count; i++) {
        Vector2 temp     = particles[i].current_pos;
        Vector2 velocity = {
            .x = particles[i].current_pos.x - particles[i].previous_pos.x,
            .y = particles[i].current_pos.y - particles[i].previous_pos.y
        };

        //      next_pos            (2 * current_pos) + current_pos - previous_pos + (acc * dt ** 2)
        //          |                                   |                                   |
        particles[i].current_pos.x = particles[i].current_pos.x + velocity.x + (particles[i].acc.x * dt * dt);
        particles[i].current_pos.y = particles[i].current_pos.y + velocity.y + (particles[i].acc.y * dt * dt);

        particles[i].previous_pos = temp;
    }
}

void Particle_Bound(Particle *particles, size_t count, float res, float fric, int screen_width, int screen_height) {
    for (size_t i = 0; i < count; i++) {
        Particle *p = &particles[i];
        Vector2 v;

        if (p->current_pos.x - p->radius < 0) {
            v = (Vector2) { p->current_pos.x - p->previous_pos.x, p->current_pos.y - p->previous_pos.y };
            p->current_pos.x  = p->radius;
            p->previous_pos.x = p->current_pos.x + v.x * res;
            p->previous_pos.y = p->current_pos.y - v.y * fric;
        }
        if (p->current_pos.x + p->radius > screen_width) {
            v = (Vector2) { p->current_pos.x - p->previous_pos.x, p->current_pos.y - p->previous_pos.y };
            p->current_pos.x  = screen_width - p->radius;
            p->previous_pos.x = p->current_pos.x + v.x * res;
            p->previous_pos.y = p->current_pos.y - v.y * fric;
        }
        if (p->current_pos.y - p->radius < 0) {
            v = (Vector2) { p->current_pos.x - p->previous_pos.x, p->current_pos.y - p->previous_pos.y };
            p->current_pos.y  = p->radius;
            p->previous_pos.y = p->current_pos.y + v.y * res;
            p->previous_pos.x = p->current_pos.x - v.x * fric;
        }
        if (p->current_pos.y + p->radius > screen_height) {
            v = (Vector2) { p->current_pos.x - p->previous_pos.x, p->current_pos.y - p->previous_pos.y };
            p->current_pos.y  = screen_height - p->radius;
            p->previous_pos.y = p->current_pos.y + v.y * res;
            p->previous_pos.x = p->current_pos.x - v.x * fric;
        }
    }
}

// =============================================
// ==========    PARTICLE ARRAY  ===============
// =============================================

Particle_Array *Particle_Array_Create(size_t capacity) {
    Particle_Array *array = malloc(sizeof(Particle_Array));
    if (!array) return NULL;

    array->count    = 0;
    array->capacity = (capacity) ? capacity : DEFAULT_CAPACITY;
    array->items    = malloc(sizeof(Particle) * array->capacity);

    if (!array->items) { free(array); return NULL; }
    return array;
}

void Particle_Array_Push(Particle_Array *array, Particle particle) {
    if (array) {
        if (array->count == array->capacity) {
            size_t new_cap = (array->capacity) ? array->capacity * 2 : DEFAULT_CAPACITY;
            Particle *temp = realloc(array->items, sizeof(Particle) * new_cap);
            if (!temp) return;

            array->items    = temp;
            array->capacity = new_cap;
        }

        array->items[array->count++] = particle;
    }
}

void Particle_Array_destroy(Particle_Array *array) {
    if (array) {
        if (array->items) { free(array->items); }
        free(array);
    }
}

void Particle_Array_MassSpawn(Particle_Array *array, size_t n_particles, Vector2 force, float radius, int screen_width, int screen_height) {
    if (!array || !array->items) return;

    int r = (int) radius;
    float v = 5.f / SUB_STEPS;   // or a fixed small value, e.g. 0.6f

    for (size_t i = 0; i < n_particles; i++) {
        float x = (float) GetRandomValue(r, screen_width  - r);
        float y = (float) GetRandomValue(r, screen_height - r);

        Particle particle = {
            .acc          = force,
            .current_pos  = (Vector2) { x, y },
            // Verlet velocity = current - previous, so keep it small (px per frame)
            .previous_pos = (Vector2) {
                x - (float) GetRandomValue(-100, 100) / 100.f * v,
                y - (float) GetRandomValue(-100, 100) / 100.f * v
            },
            .radius       = radius,
            .inv_mass     = 1.f / ((radius > 0.f) ? (radius * radius) : 1.f),
        };

        Particle_Array_Push(array, particle);
    }
}

#endif

#endif