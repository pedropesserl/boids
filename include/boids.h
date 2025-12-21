#ifndef BOIDS_H_
#define BOIDS_H_

#include <stdio.h>
#include "raylib.h"

#define MEM_ERR do {                                                                    \
        fprintf(stderr, "%s:%d (%s): Mem alloc error\n", __FILE__, __LINE__, __func__); \
        exit(1);                                                                        \
    } while (0)

#define COLOR_BACKGROUND BLACK
#define COLOR_BOID RED
#define BOID_SIZE 4.0f
#define BOID_MAX_SPEED 10.0f
#define BOID_FOV_RADIUS 100.0f
#define SEPARATION_SCALE_FACTOR 0.6f
#define ALIGNMENT_SCALE_FACTOR 0.125f
#define COHESION_SCALE_FACTOR 0.05f

typedef struct Boid {
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
} Boid;

typedef struct Boids {
    size_t count;
    size_t capacity;
    Boid *items;
} Boids;

Boid create_boid(void);
void insert_boid(Boid boid, Boids *boids);
void destroy_boids(Boids *boidls);
void update_boids(Boids *boids);
void draw_boids(Boids *boids);

#endif // BOIDS_H_
