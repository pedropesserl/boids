#ifndef BOIDS_UI_H_
#define BOIDS_UI_H_

#include "raylib.h"
#include "boids.h"

#define SLIDER_BAR_WIDTH 200.0f
#define SLIDER_BAR_HEIGHT 8.0f
#define SLIDER_BALL_RADIUS SLIDER_BAR_HEIGHT
#define COLOR_SLIDER_BAR GRAY
#define COLOR_SLIDER_BALL RED

#define LABEL_FONT_SIZE (2.5f * SLIDER_BAR_HEIGHT)
#define MAX_LABEL_CHARACTERS 11
#define COLOR_LABEL RAYWHITE

#define RESET_BUTTON_WIDTH 100.0f
#define RESET_BUTTON_PAD SLIDER_BAR_HEIGHT
#define RESET_BUTTON_HEIGHT (LABEL_FONT_SIZE + 2.0f * RESET_BUTTON_PAD)
#define COLOR_RESET_BUTTON BLACK
#define COLOR_RESET_BUTTON_BORDER RAYWHITE

typedef struct Slider {
    float value; // 0 <= value <= 1
    Vector2 position;
    Vector2 ball_position;
    bool is_dragged;
    char name_label[MAX_LABEL_CHARACTERS];
    char value_label[MAX_LABEL_CHARACTERS];
} Slider;

typedef struct Button {
    Rectangle box;
    bool is_pressed;
    float roundness;
    Color color;
    bool has_border;
    float border_thickness;
    Color border_color;
    char label[MAX_LABEL_CHARACTERS];
} Button;

typedef struct UI {
    Slider fov_radius_slider;
    Slider separation_slider;
    Slider alignment_slider;
    Slider cohesion_slider;
    Button reset_button;
} UI;

UI create_ui(void);
void update_ui(UI *ui);
void update_boids_config(Boids *boids, UI *ui);
void draw_ui(UI *ui);

#endif // BOIDS_UI_H_
