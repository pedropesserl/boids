#include <stdlib.h>
#include <stdbool.h>
#include "boids.h"
#include "raymath.h"

static float random_float(float min, float max) {
    float scale = (float)rand() / (float)RAND_MAX;
    return min + scale * (max - min);
}

static Vector2 random_vector2(float magnitude) {
    Vector2 rand = {random_float(-1, 1), random_float(-1, 1)};
    return Vector2Scale(Vector2Normalize(rand), magnitude);
}

static bool is_in_flock(Boid *other, Boid *this, Boids *boids) {
    Vector2 difference = Vector2Subtract(other->position, this->position);
    float distance = Vector2Length(difference);
    float dot = Vector2DotProduct(Vector2Normalize(this->velocity), Vector2Normalize(difference));
    // -1 <= dot <= 1  <=>  0 <= dot + 1 <= 2  <=>  0 <= (dot + 1) / 2 <= 1
    float percentage = 1.0f - (dot + 1.0f) / 2.0f; // dot = -1 => percentage = 100% ; dot = 1 => percentage = 0%
    return distance < boids->fov_radius && percentage < boids->fov_percentage;
}

static Vector2 wrap_around_edges(Vector2 position) {
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

static Vector2 separation(Boids *boids, size_t index_boid) {
    Boid *this = &boids->items[index_boid];
    size_t flock_size = 0;
    Vector2 separation = Vector2Zero();
    for (size_t i = 0; i < boids->count; i++) {
        Boid *other = &boids->items[i];
        if (i != index_boid && is_in_flock(other, this, boids)) {
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
    return Vector2Scale(separation, boids->separation_intensity);
}

static Vector2 alignment(Boids *boids, size_t index_boid) {
    Boid *this = &boids->items[index_boid];
    size_t flock_size = 0;
    Vector2 alignment = Vector2Zero();
    for (size_t i = 0; i < boids->count; i++) {
        Boid *other = &boids->items[i];
        if (i != index_boid && is_in_flock(other, this, boids)) {
            alignment = Vector2Add(alignment, other->velocity);
            flock_size++;
        }
    }
    if (flock_size > 0) {
        alignment = Vector2Scale(alignment, 1.0f / (float)flock_size);
    }
    return Vector2Scale(alignment, boids->alignment_intensity);
}

static Vector2 cohesion(Boids *boids, size_t index_boid) {
    Boid *this = &boids->items[index_boid];
    size_t flock_size = 0;
    Vector2 cohesion = Vector2Zero();
    for (size_t i = 0; i < boids->count; i++) {
        Boid *other = &boids->items[i];
        if (i != index_boid && is_in_flock(other, this, boids)) {
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
    return Vector2Scale(cohesion, boids->cohesion_intensity);
}

static void draw_boid(Boid boid, Color color) {
    Vector2 direction = Vector2Normalize(boid.velocity);
    Vector2 boid_vector = Vector2Scale(direction, BOID_SIZE);
    Vector2 vertex1 = Vector2Add(boid.position, Vector2Scale(boid_vector, 2.0f));
    Vector2 vertex2 = Vector2Add(boid.position, Vector2Rotate(boid_vector, -2 * PI / 3));
    Vector2 vertex3 = Vector2Add(boid.position, Vector2Rotate(boid_vector, 2 * PI / 3));
    DrawTriangle(vertex1, vertex2, vertex3, color);
}

static Boid create_boid(void) {
    return (Boid){
        .position = { GetRandomValue(0, GetScreenWidth()),
                      GetRandomValue(0, GetScreenHeight()) },
        .velocity = random_vector2(GetRandomValue(0, BOID_MAX_SPEED)),
        .acceleration = Vector2Zero(),
    };
}

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

Boids create_boids(int num_boids) {
    Boids boids = {
        .count = 0,
        .capacity = 0,
        .items = NULL,
        .fov_radius = BOID_BASE_FOV_RADIUS,
        .fov_percentage = 0.75f,
        .separation_intensity = BOID_BASE_SEPARATION_INTENSITY,
        .alignment_intensity = BOID_BASE_ALIGNMENT_INTENSITY,
        .cohesion_intensity = BOID_BASE_COHESION_INTENSITY,
    };
    for (int i = 0; i < num_boids; i++) {
        insert_boid(create_boid(), &boids);
    }
    return boids;
}

void destroy_boids(Boids *boids) {
    if (boids->capacity > 0) {
        free(boids->items);
    }
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

void draw_boids(Boids *boids) {
    for (size_t i = 0; i < boids->count; i++) {
        draw_boid(boids->items[i], RED);
    }
}
