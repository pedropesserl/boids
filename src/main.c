#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "raylib.h"
#include "raymath.h"

#define MEM_ERR do {                                                                    \
        fprintf(stderr, "%s:%d (%s): Mem alloc error\n", __FILE__, __LINE__, __func__); \
        exit(1);                                                                        \
    } while (0)

#define COLOR_BACKGROUND BLACK
#define COLOR_BOID RED
#define NUM_BOIDS 60
#define BOID_SIZE 8.0f
#define BOID_MAX_SPEED 4.0f
#define BOID_FOV_RADIUS 100.0f
#define SEPARATION_SCALE_FACTOR 0.9f
#define ALIGNMENT_SCALE_FACTOR 0.015f
#define COHESION_SCALE_FACTOR 0.009f

float random_float(float min, float max) {
    float scale = (float)rand() / (float)RAND_MAX;
    return min + scale * (max - min);
}

Vector2 random_vector2(float magnitude) {
    Vector2 rand = {random_float(-1, 1), random_float(-1, 1)};
    return Vector2Scale(Vector2Normalize(rand), magnitude);
}

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

void insert_boid(Boid boid, Boids *boids) {
    if (boids->capacity == 0) {
        boids->items = (Boid*)calloc(1, sizeof(Boid));
        if (!boids->items) {
            MEM_ERR;
        }
        boids->capacity = 1;
    }
    if (boids->count == boids->capacity) {
        boids->capacity *= 2;
        boids->items = reallocarray(boids->items, boids->capacity, sizeof(Boid));
    }
    boids->items[boids->count] = boid;
    boids->count++;
}

void destroy_boids(Boids *boids) {
    if (boids->capacity > 0) {
        free(boids->items);
    }
}

Vector2 steer_towards(Vector2 target, Vector2 current) {
    
}

bool is_in_flock(Boid *other, Boid *this) {
    Vector2 difference = Vector2Subtract(other->position, this->position);
    float dot = Vector2DotProduct(Vector2Normalize(this->velocity), Vector2Normalize(difference));
    float distance = Vector2Length(difference);
    return distance < BOID_FOV_RADIUS && dot > -0.75f;
}

Vector2 separation(Boids *boids, size_t index_boid) {
    Boid *this = &boids->items[index_boid];
    size_t flock_size = 0;
    Vector2 avg_repelling = Vector2Zero();
    for (size_t i = 0; i < boids->count; i++) {
        Boid *other = &boids->items[i];
        if (i != index_boid && is_in_flock(other, this)) {
            Vector2 difference = Vector2Subtract(other->position, this->position);
            float distance = Vector2Length(difference);
            Vector2 repelling = Vector2Scale(difference, 1.0f / (distance));
            avg_repelling = Vector2Add(avg_repelling, repelling);
            flock_size++;
        }
    }
    if (flock_size > 0) {
        avg_repelling = Vector2Scale(avg_repelling, 1.0f / (float)flock_size);
    } else {
        return Vector2Zero();
    }
    Vector2 separation = avg_repelling;
    return Vector2Scale(separation, SEPARATION_SCALE_FACTOR);
}

Vector2 alignment(Boids *boids, size_t index_boid) {
    Boid *this = &boids->items[index_boid];
    size_t flock_size = 0;
    Vector2 avg_velocity = Vector2Zero();
    for (size_t i = 0; i < boids->count; i++) {
        Boid *other = &boids->items[i];
        if (i != index_boid && is_in_flock(other, this)) {
            avg_velocity = Vector2Add(avg_velocity, other->velocity);
            flock_size++;
        }
    }
    if (flock_size > 0) {
        avg_velocity = Vector2Scale(avg_velocity, 1.0f / (float)flock_size);
    } else {
        return Vector2Zero();
    }
    Vector2 alignment = Vector2Subtract(avg_velocity, this->velocity);
    return Vector2Scale(alignment, ALIGNMENT_SCALE_FACTOR);
}

Vector2 cohesion(Boids *boids, size_t index_boid) {
    Boid *this = &boids->items[index_boid];
    size_t flock_size = 0;
    Vector2 avg_position = Vector2Zero();
    for (size_t i = 0; i < boids->count; i++) {
        Boid *other = &boids->items[i];
        if (i != index_boid && is_in_flock(other, this)) {
            avg_position = Vector2Add(avg_position, other->position);
            flock_size++;
        }
    }
    if (flock_size > 0) {
        avg_position = Vector2Scale(avg_position, 1.0f / (float)flock_size);
    } else {
        return Vector2Zero();
    }
    Vector2 cohesion = Vector2Subtract(avg_position, this->position);
    return Vector2Scale(cohesion, COHESION_SCALE_FACTOR);
}

void update_boids(Boids *boids) {
    for (size_t i = 0; i < boids->count; i++) {
        Vector2 acceleration = Vector2Zero();
        acceleration = Vector2Add(acceleration, separation(boids, i));
        acceleration = Vector2Add(acceleration, alignment(boids, i));
        acceleration = Vector2Add(acceleration, cohesion(boids, i));
        boids->items[i].acceleration = acceleration;
    }
    for (size_t i = 0; i < boids->count; i++) {
        Boid *boid = &boids->items[i];
        Vector2 new_velocity = Vector2Add(boid->velocity, boid->acceleration);
        boid->velocity = Vector2Scale(Vector2Normalize(new_velocity), BOID_MAX_SPEED);
        boid->position = Vector2Add(boid->position, boid->velocity);
        Vector2 screen_limit = {GetScreenWidth(), GetScreenHeight()};
        if (boid->position.x < 0) {
            boid->position.x = screen_limit.x;
        } else if (boid->position.x > screen_limit.x) {
            boid->position.x = 0;
        }
        if (boid->position.y < 0) {
            boid->position.y = screen_limit.y;
        } else if (boid->position.y > screen_limit.y) {
            boid->position.y = 0;
        }
    }
}

void draw_boid(Boid boid, Color color) {
    //      v1
    //     .  .
    //    .    .
    //   .      .
    //  v2 .... v3

    Vector2 direction = Vector2Normalize(boid.velocity);
    Vector2 boid_vector = Vector2Scale(direction, BOID_SIZE);
    Vector2 vertex1 = Vector2Add(boid.position, Vector2Scale(boid_vector, 2.0f));
    Vector2 vertex2 = Vector2Add(boid.position, Vector2Rotate(boid_vector, -2 * PI / 3));
    Vector2 vertex3 = Vector2Add(boid.position, Vector2Rotate(boid_vector, 2 * PI / 3));
    DrawTriangle(vertex1, vertex2, vertex3, color);
}

void draw_boids(Boids *boids) {
    for (size_t i = 0; i < boids->count; i++) {
        draw_boid(boids->items[i], RED);
    }
    for (size_t i = 1; i < boids->count; i++) {
        if (is_in_flock(&boids->items[i], &boids->items[0])) {
            draw_boid(boids->items[i], GREEN);
        }
    }
    DrawCircleLinesV(boids->items[0].position, BOID_FOV_RADIUS, GREEN);
}

int main() {
    InitWindow(1200, 900, "boids");
    SetTargetFPS(60);
    SetRandomSeed(time(0));

    Boids boids;
    for (int i = 0; i < NUM_BOIDS; i++) {
        insert_boid((Boid){
            .position = { GetRandomValue(0, GetScreenWidth()),
                          GetRandomValue(0, GetScreenHeight()) },
            .velocity = random_vector2(BOID_MAX_SPEED),
            .acceleration = Vector2Zero(),
        }, &boids);
    }

    while (!WindowShouldClose()) {
        update_boids(&boids);
        BeginDrawing();
        ClearBackground(BLACK);
        draw_boids(&boids);
        EndDrawing();
    }
    CloseWindow();

    destroy_boids(&boids);

    return 0;
}
