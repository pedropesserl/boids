#include "raylib.h"
#include "raymath.h"
#include "boids_ui.h"

static Button create_reset_button(Vector2 position) {
    return (Button){
        .box = {
            .x = position.x,
            .y = position.y,
            .width = RESET_BUTTON_WIDTH,
            .height = RESET_BUTTON_HEIGHT,
        },
        .is_pressed = false,
        .roundness = 0.3f,
        .color = COLOR_RESET_BUTTON,
        .has_border = true,
        .border_thickness = 1.0f,
        .border_color = COLOR_RESET_BUTTON_BORDER,
        .label = "Reset",
    };
}

static Slider create_slider(const char *name, Vector2 position) {
    Slider s = {
        .value = 0.5f,
        .position = position,
        .ball_position = {
            .x = Lerp(position.x, position.x + SLIDER_BAR_WIDTH, 0.5f),
            .y = position.y + LABEL_FONT_SIZE + SLIDER_BAR_HEIGHT,
        },
        .is_dragged = false,
        .name_label = {0},
        .value_label = {0},
    };
    snprintf(s.name_label, MAX_LABEL_CHARACTERS, "%s", name);
    return s;
}

UI create_ui(void) {
    float margin = 20.0f;
    float pos_x = margin, pos_y = margin;
    float spacing = margin + LABEL_FONT_SIZE + 1.5f * SLIDER_BAR_HEIGHT;
    int iota = 0;
    return (UI){
        .fov_radius_slider = create_slider("FOV Radius", (Vector2){pos_x, pos_y + spacing * iota++}),
        .separation_slider = create_slider("Separation", (Vector2){pos_x, pos_y + spacing * iota++}),
        .alignment_slider  = create_slider("Alignment",  (Vector2){pos_x, pos_y + spacing * iota++}),
        .cohesion_slider   = create_slider("Cohesion",   (Vector2){pos_x, pos_y + spacing * iota++}),
        .reset_button = create_reset_button((Vector2){pos_x, pos_y + spacing * iota++}),
    };
}

static bool update_slider(Slider *slider, Vector2 mouse, bool reset, float base_value) {
    if (reset) {
        slider->value = 0.5f;
        slider->ball_position.x = Lerp(slider->position.x, slider->position.x + SLIDER_BAR_WIDTH, 0.5f);
    }
    bool is_hovered = false, is_dragged = false;
    if (CheckCollisionPointCircle(mouse, slider->ball_position, SLIDER_BALL_RADIUS)) {
        is_hovered = true;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            slider->is_dragged = true;
        }
    }
    if (slider->is_dragged) {
        is_dragged = true;
        float mouse_x = Clamp(mouse.x, slider->position.x, slider->position.x + SLIDER_BAR_WIDTH);
        slider->value = (mouse_x - slider->position.x) / SLIDER_BAR_WIDTH;
        slider->ball_position.x = mouse_x;
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            slider->is_dragged = false;
        }
    }
    snprintf(slider->value_label, MAX_LABEL_CHARACTERS, "%.3f", slider->value * 2.0f * base_value);
    return is_hovered || is_dragged;
}

static bool update_reset_button(Button *reset, Vector2 mouse) {
    bool is_hovered = false;
    if (reset->is_pressed) {
        reset->is_pressed = false;
    }
    if (CheckCollisionPointRec(mouse, reset->box)) {
        is_hovered = true;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            reset->is_pressed = true;
        }
    }
    return is_hovered;
}

void update_ui(UI *ui) {
    Vector2 mouse = GetMousePosition();
    bool should_point = false;
    should_point |= update_reset_button(&ui->reset_button, mouse);
    bool reset = ui->reset_button.is_pressed;
    should_point |= update_slider(&ui->fov_radius_slider, mouse, reset, BOID_BASE_FOV_RADIUS);
    should_point |= update_slider(&ui->separation_slider, mouse, reset, BOID_BASE_SEPARATION_INTENSITY);
    should_point |= update_slider(&ui->alignment_slider,  mouse, reset, BOID_BASE_ALIGNMENT_INTENSITY);
    should_point |= update_slider(&ui->cohesion_slider,   mouse, reset, BOID_BASE_COHESION_INTENSITY);
    if (should_point) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    } else {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
}

void update_boids_config(Boids *boids, UI *ui) {
    boids->fov_radius = ui->fov_radius_slider.value * 2.0f * BOID_BASE_FOV_RADIUS;
    boids->separation_intensity = ui->separation_slider.value * 2.0f * BOID_BASE_SEPARATION_INTENSITY;
    boids->alignment_intensity = ui->alignment_slider.value * 2.0f * BOID_BASE_ALIGNMENT_INTENSITY;
    boids->cohesion_intensity = ui->cohesion_slider.value * 2.0f * BOID_BASE_COHESION_INTENSITY;
}

static void draw_slider(Slider *slider) {
    Rectangle bar = {
        .x = slider->position.x,
        .y = slider->position.y + LABEL_FONT_SIZE + SLIDER_BAR_HEIGHT / 2.0f,
        .width = SLIDER_BAR_WIDTH,
        .height = SLIDER_BAR_HEIGHT,
    };
    Vector2 ball_position = {
        .x = Lerp(slider->position.x, slider->position.x + SLIDER_BAR_WIDTH, slider->value),
        .y = bar.y + SLIDER_BAR_HEIGHT / 2.0f,
    };
    DrawRectangleRounded(bar, 1.0f, 10, COLOR_SLIDER_BAR);
    DrawCircleV(ball_position, SLIDER_BALL_RADIUS, COLOR_SLIDER_BALL);
    DrawText(slider->name_label, slider->position.x, slider->position.y, LABEL_FONT_SIZE, COLOR_LABEL);
    DrawText(slider->value_label, bar.x + SLIDER_BAR_WIDTH + 10, bar.y - SLIDER_BAR_HEIGHT / 2.0f, LABEL_FONT_SIZE, COLOR_LABEL);
}

static void draw_button(Button *button) {
    const int segments = 20;
    DrawRectangleRounded(button->box, button->roundness, segments, button->color);
    if (button->has_border) {
        DrawRectangleRoundedLinesEx(button->box, button->roundness, segments, button->border_thickness, button->border_color);
    }
    int label_width = MeasureText(button->label, LABEL_FONT_SIZE);
    DrawText(button->label, button->box.x + button->box.width / 2.0f - label_width / 2.0f, button->box.y + RESET_BUTTON_PAD, LABEL_FONT_SIZE, COLOR_RESET_BUTTON_BORDER);
}

void draw_ui(UI *ui) {
    draw_slider(&ui->fov_radius_slider);
    draw_slider(&ui->separation_slider);
    draw_slider(&ui->alignment_slider);
    draw_slider(&ui->cohesion_slider);
    draw_button(&ui->reset_button);
}
