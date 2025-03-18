#include "raylib.h"
#include "stdlib.h"
#include "math.h"
#include "stdio.h"

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define WINDOW_TITLE "fuck raylib <3"

#define NUM_OBJECTS 77
#define PLAYER_SIZE 9

typedef struct {
    float x;
    float y;
    float speed;
} Player;

typedef struct {
    bool active;
    Vector2 position;
    Color color;
    float current_length;
    float max_length;
    float expansion_speed;
    float duration;
    float elapsed_time;
    float thickness;
} ClickAnimation;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float speed;
    float radius;
} Projectile;

typedef struct {
    bool active;
    Vector2 position;
    float radius;
    Color color;
    int health;
    int max_health;
} Object;

void init_objects(Object objects[], int count);
void draw_objects(const Object objects[], int count);

void add_projectile(Projectile **projectiles, int *count, int *capacity, Projectile proj);
void update_projectile(Projectile *proj, float delta_time);

void init_click_animation(ClickAnimation *anim, Vector2 position);
void update_click_animation(ClickAnimation *anim, float delta_time);
void draw_click_animation(const ClickAnimation *anim);

int main(int argc, char **argv) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(165);

    Player player = { 0, 0, 512 };
    Vector2 target = { player.x, player.y };

    ClickAnimation click_anim = { 0 };
    Projectile *projectiles = NULL;
    Object objects[NUM_OBJECTS];

    int projectile_count = 0;
    int projectile_capacity = 0;

    float fire_cooldown = 0.0f;
    float fire_rate = 0.0f;

    int active_objects = NUM_OBJECTS;

    init_objects(objects, NUM_OBJECTS);
    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        if (IsKeyDown(KEY_R)) {
            init_objects(objects, NUM_OBJECTS);
            active_objects = NUM_OBJECTS;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            target = GetMousePosition();
            init_click_animation(&click_anim, target);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            target = GetMousePosition();
        }

        fire_cooldown -= delta_time;
        if (fire_cooldown < 0) fire_cooldown = 0;

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && fire_cooldown <= 0.0f) {
            Vector2 mouse_pos = GetMousePosition();
            Vector2 direction = { mouse_pos.x - player.x, mouse_pos.y - player.y };

            float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
            if (length != 0) {
                direction.x /= length;
                direction.y /= length;

                Projectile new_proj;
                new_proj.speed = 1200.0f;
                new_proj.radius = 3.0f;
                new_proj.position = (Vector2){ player.x, player.y };
                new_proj.velocity.x = direction.x * new_proj.speed;
                new_proj.velocity.y = direction.y * new_proj.speed;

                add_projectile(&projectiles, &projectile_count, &projectile_capacity, new_proj);
            }

            fire_cooldown = fire_rate;
        }

        Vector2 move_dir = { target.x - player.x, target.y - player.y };
        float distance = sqrtf(move_dir.x * move_dir.x + move_dir.y * move_dir.y);
        if (distance > player.speed * delta_time) {
            move_dir.x /= distance;
            move_dir.y /= distance;
            player.x += move_dir.x * player.speed * delta_time;
            player.y += move_dir.y * player.speed * delta_time;
        } else {
            player.x = target.x;
            player.y = target.y;
        }
        player.x = fmaxf(PLAYER_SIZE + 5, fminf(player.x, WINDOW_WIDTH - PLAYER_SIZE - 5));
        player.y = fmaxf(PLAYER_SIZE + 5, fminf(player.y, WINDOW_HEIGHT - PLAYER_SIZE - 5));

        update_click_animation(&click_anim, delta_time);
        for (int i = 0; i < projectile_count; ) {
            Projectile *proj = &projectiles[i];
            update_projectile(proj, delta_time);

            bool collided = false;
            for (int j = 0; j < NUM_OBJECTS; j++) {
                if (!(objects[j].active && CheckCollisionCircles(proj->position, proj->radius, objects[j].position, objects[j].radius))) continue;

                if (objects[j].health > 1) {
                    objects[j].health--;
                } else {
                    objects[j].active = false;
                    active_objects--;
                }

                collided = true;
                break;
            }

            if (collided || proj->position.x < 0 || proj->position.x > WINDOW_WIDTH || proj->position.y < 0 || proj->position.y > WINDOW_HEIGHT) {
                projectiles[i] = projectiles[projectile_count - 1];
                projectile_count--;
            } else i++;
        }

        BeginDrawing();
            ClearBackground(BLACK);

            draw_objects(objects, NUM_OBJECTS);

            for (int i = 0; i < projectile_count; i++) {
                DrawCircleV(projectiles[i].position, projectiles[i].radius, RED);
            }

            draw_click_animation(&click_anim);

            DrawCircle(player.x, player.y, PLAYER_SIZE + 3, Fade(DARKGRAY, 0.3f));
            DrawCircle(player.x, player.y, PLAYER_SIZE, DARKGRAY);

            DrawFPS(10, 10);
            DrawText(TextFormat("Active objects: %d", active_objects), 10, 40, 20, RED);
        EndDrawing();
    }

    free(projectiles);
    CloseWindow();
    return 0;
}

void init_objects(Object objects[], int count) {
    for (int i = 0; i < count; i++) {
        int health = GetRandomValue(7, 13);
        objects[i].radius = (float)health;

        bool valid_position = false;
        while (!valid_position) {
            objects[i].position.x = GetRandomValue(50, WINDOW_WIDTH - 50);
            objects[i].position.y = GetRandomValue(50, WINDOW_HEIGHT - 50);

            valid_position = true;
            for (int j = 0; j < i; j++) {
                if (!(CheckCollisionCircles(objects[i].position, objects[i].radius, objects[j].position, objects[j].radius))) continue;

                valid_position = false;
                break;
            }
        }

        objects[i].active = true;
        objects[i].health = health;
        objects[i].max_health = health;

        Color rand_color = {
            GetRandomValue(0, 255),
            GetRandomValue(0, 255),
            GetRandomValue(0, 255),
            255
        };
        objects[i].color = Fade(rand_color, 0.77f);
    }
}

void draw_objects(const Object objects[], int count) {
    for (int i = 0; i < count; i++) {
        if (!objects[i].active) continue;

        DrawCircleV(objects[i].position, objects[i].radius, objects[i].color);

        const char *health_text = TextFormat("%d/%d HP", objects[i].health, objects[i].max_health);
        int text_width = MeasureText(health_text, objects[i].radius);
        Vector2 text_position = {
            objects[i].position.x - text_width / 2,
            objects[i].position.y - objects[i].radius - 17
        };

        DrawText(health_text, text_position.x, text_position.y, objects[i].radius, objects[i].color);
    }
}

void add_projectile(Projectile **projectiles, int *count, int *capacity, Projectile proj) {
    if (*count >= *capacity) {
        *capacity = (*capacity == 0) ? 4 : *capacity * 2;
        *projectiles = realloc(*projectiles, *capacity * sizeof(Projectile));
    }

    (*projectiles)[*count] = proj;
    (*count)++;
}

void update_projectile(Projectile *proj, float delta_time) {
    proj->position.x += proj->velocity.x * delta_time;
    proj->position.y += proj->velocity.y * delta_time;
}

void init_click_animation(ClickAnimation *anim, Vector2 position) {
    anim->position = position;
    anim->active = true;
    anim->current_length = 0.0f;
    anim->max_length = 30.0f;
    anim->thickness = 1.5f;
    anim->expansion_speed = 150.0f;
    anim->duration = anim->max_length / anim->expansion_speed;
    anim->elapsed_time = 0.0f;
    anim->color = GREEN;
}

void update_click_animation(ClickAnimation *anim, float delta_time) {
    if (!anim->active) return;

    anim->elapsed_time += delta_time;
    anim->current_length = anim->expansion_speed * anim->elapsed_time;

    if (anim->elapsed_time >= anim->duration) anim->active = false;
}

void draw_click_animation(const ClickAnimation *anim) {
    if (!anim->active) return;

    float angles[4] = { 45.0f, 135.0f, 225.0f, 315.0f };
    for (int i = 0; i < 4; i++) {
        float angle_rad = DEG2RAD * angles[i];

        Vector2 end_pos = {
            anim->position.x + cosf(angle_rad) * (anim->current_length - anim->max_length),
            anim->position.y + sinf(angle_rad) * (anim->current_length - anim->max_length)
        };

        DrawLineEx(anim->position, end_pos, anim->thickness, anim->color);
    }
}
