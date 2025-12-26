#include <time.h>
#include "boids.h"
#include "boids_ui.h"

int main() {
    SetRandomSeed(time(0));
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1200, 900, "boids");
    SetTargetFPS(60);

    Boids boids = create_boids(500);
    UI ui = create_ui();

    while (!WindowShouldClose()) {
        update_boids(&boids);
        update_ui(&ui);
        update_boids_config(&boids, &ui);

        BeginDrawing();
        ClearBackground(BLACK);
        draw_boids(&boids);
        draw_ui(&ui);
        EndDrawing();
    }
    CloseWindow();

    destroy_boids(&boids);

    return 0;
}
