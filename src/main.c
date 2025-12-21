#include <time.h>
#include "boids.h"

int main() {
    InitWindow(1200, 900, "boids");
    SetTargetFPS(60);
    SetRandomSeed(time(0));

    Boids boids = {0};
    int num_boids = 500;
    for (int i = 0; i < num_boids; i++) {
        insert_boid(create_boid(), &boids);
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
