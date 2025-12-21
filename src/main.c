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
#define NUM_BOIDS 500
#define BOID_SIZE 4.0f
#define BOID_MAX_SPEED 8.0f
#define BOID_FOV_RADIUS 100.0f
#define SEPARATION_SCALE_FACTOR 0.6f
#define ALIGNMENT_SCALE_FACTOR 0.125f
#define COHESION_SCALE_FACTOR 0.05f

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

Vector2 wrap_around_edges(Vector2 position) {
    Vector2 screen_limit = {GetScreenWidth(), GetScreenHeight()};
    if (position.x < 0) {
        position.x = screen_limit.x;
    } else if (position.x > screen_limit.x) {
        position.x = 0;
    }
    if (position.y < 0) {
        position.y = screen_limit.y;
    } else if (position.y > screen_limit.y) {
        position.y = 0;
    }
    return position;
}

Vector2 separation(Boids *boids, size_t index_boid) {
    Boid *this = &boids->items[index_boid];
    size_t flock_size = 0;
    Vector2 separation = Vector2Zero();
    for (size_t i = 0; i < boids->count; i++) {
        Boid *other = &boids->items[i];
        if (i != index_boid && is_in_flock(other, this)) {
            Vector2 difference = Vector2Subtract(this->position, other->position);
            float distance = Vector2Length(difference);
            Vector2 repelling = Vector2Scale(Vector2Normalize(difference), 1.0f / distance * distance);
            separation = Vector2Add(separation, repelling);
            flock_size++;
        }
    }
    if (flock_size > 0) {
        separation = Vector2Scale(separation, 1.0f / (float)flock_size);
    }
    return Vector2Scale(separation, SEPARATION_SCALE_FACTOR);
}

Vector2 alignment(Boids *boids, size_t index_boid) {
    Boid *this = &boids->items[index_boid];
    size_t flock_size = 0;
    Vector2 alignment = Vector2Zero();
    for (size_t i = 0; i < boids->count; i++) {
        Boid *other = &boids->items[i];
        if (i != index_boid && is_in_flock(other, this)) {
            alignment = Vector2Add(alignment, other->velocity);
            flock_size++;
        }
    }
    if (flock_size > 0) {
        alignment = Vector2Scale(alignment, 1.0f / (float)flock_size);
    }
    return Vector2Scale(alignment, ALIGNMENT_SCALE_FACTOR);
}

Vector2 cohesion(Boids *boids, size_t index_boid) {
    Boid *this = &boids->items[index_boid];
    size_t flock_size = 0;
    Vector2 cohesion = Vector2Zero();
    for (size_t i = 0; i < boids->count; i++) {
        Boid *other = &boids->items[i];
        if (i != index_boid && is_in_flock(other, this)) {
            cohesion = Vector2Add(cohesion, other->position);
            flock_size++;
        }
    }
    if (flock_size > 0) {
        cohesion = Vector2Scale(cohesion, 1.0f / (float)flock_size);
        Vector2 diff_to_target = Vector2Subtract(cohesion, this->position);
        cohesion = Vector2Scale(Vector2Normalize(diff_to_target), BOID_MAX_SPEED);
        cohesion = Vector2Subtract(cohesion, this->velocity);
    }
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
        if (Vector2Length(new_velocity) > BOID_MAX_SPEED) {
            new_velocity = Vector2Scale(Vector2Normalize(new_velocity), BOID_MAX_SPEED);
        }
        boid->velocity = new_velocity;
        boid->position = Vector2Add(boid->position, boid->velocity);
        boid->position = wrap_around_edges(boid->position);
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
    /* for (size_t i = 1; i < boids->count; i++) { */
    /*     if (is_in_flock(&boids->items[i], &boids->items[0])) { */
    /*         draw_boid(boids->items[i], GREEN); */
    /*     } */
    /* } */
    /* DrawCircleLinesV(boids->items[0].position, BOID_FOV_RADIUS, GREEN); */
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
            /* .velocity = random_vector2(GetRandomValue(0, BOID_MAX_SPEED)), */
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
