#include "VkvgApp.hpp"

#include <vector>
#include <iostream>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

#define BRICK_ROWS 5
#define BRICK_COLS 10
#define PI 3.14159265358979323846
enum PowerUpType { SPEED_BOOST, EXTRA_LIFE, GROW_PADDLE };

struct Brick {
    float x, y;
    bool active;
    int type;
};

struct PowerUp {
    float x, y;
    float speed;
    bool active;
    PowerUpType type;
};

// Fixed multi-dimensional array initializer format
const float BRICK_COLORS[5][3] = {
    {1.0f, 0.2f, 0.2f},  // Top Red Row
    {1.0f, 0.6f, 0.0f},  // Orange Row
    {0.9f, 0.9f, 0.0f},  // Yellow Row
    {0.0f, 0.9f, 0.0f},  // Green Row
    {0.0f, 0.8f, 1.0f}   // Cyan Bottom Row
};

class ArkanoidApp : public VkvgApp {

  public:
    // Paddle Dimensions & Vectors
    float paddle_x = 512.0f;
    float paddle_y = 920.0f;
    float paddle_w = 120.0f;
    float paddle_h = 16.0f;

           // Ball Dynamics
    float ball_x = 512.0f;
    float ball_y = 600.0f;
    float ball_r = 8.0f;
    float ball_dx = 3.0f;
    float ball_dy = -5.0f;

    std::vector<Brick> bricks;
    std::vector<PowerUp> falling_bonuses;

    float powerup_timer = 0.0f;
    bool has_active_powerup = false;

    int score = 0;
    int lives = 3;
    bool game_over = false;
    bool victory = false;

    void Init() override {
        VkvgApp::Init();
        srand((unsigned int)time(NULL));

        paddle_x = width / 2.0f;
        paddle_y = height * 0.90f;
        reset_ball();

        float grid_padding_left = 62.0f;
        float brick_w = 90.0f;
        float brick_h = 24.0f;
        float brick_gap = 4.0f;

        for (int r = 0; r < BRICK_ROWS; r++) {
            for (int c = 0; c < BRICK_COLS; c++) {
                Brick bk;
                bk.x = grid_padding_left + c * (brick_w + brick_gap);
                bk.y = 120.0f + r * (brick_h + brick_gap);
                bk.active = true;
                bk.type = r;
                bricks.push_back(bk);
            }
        }
    }

    void reset_ball() {
        ball_x = paddle_x;
        ball_y = paddle_y - 20.0f;
        ball_dx = (rand() % 2 == 0) ? 3.0f : -3.0f;
        ball_dy = -5.0f;
    }

    void Update() override {
        if (game_over || victory) return;

               // Move Ball
        ball_x += ball_dx;
        ball_y += ball_dy;

               // Side Wall Collisions
        if (ball_x - ball_r <= 0) {
            ball_x = ball_r;
            ball_dx *= -1.0f;
        } else if (ball_x + ball_r >= width) {
            ball_x = width - ball_r;
            ball_dx *= -1.0f;
        }

        // Ceiling Bounce
        if (ball_y - ball_r <= 0) {
            ball_y = ball_r;
            ball_dy *= -1.0f;
        }

               // Floor Death Zone
        if (ball_y + ball_r >= height) {
            lives--;
            if (lives <= 0) {
                game_over = true;
            } else {
                reset_ball();
            }
            return;
        }

               // Paddle Intersections
        float pad_left = paddle_x - (paddle_w / 2.0f);
        float pad_right = paddle_x + (paddle_w / 2.0f);

        if (ball_y + ball_r >= paddle_y && ball_y - ball_r <= paddle_y + paddle_h &&
            ball_x >= pad_left && ball_x <= pad_right) {

            ball_y = paddle_y - ball_r;
            ball_dy *= -1.0f;

                   // Stable angle transformation engine
            float hit_point_delta = (ball_x - paddle_x) / (paddle_w / 2.0f);
            ball_dx = hit_point_delta * 5.0f;
        }

               // Brick Collisions
        float brick_w = 90.0f;
        float brick_h = 24.0f;
        bool all_destroyed = true;

        for (auto& bk : bricks) {
            if (!bk.active) continue;
            all_destroyed = false;

            if (ball_x + ball_r >= bk.x && ball_x - ball_r <= bk.x + brick_w &&
                ball_y + ball_r >= bk.y && ball_y - ball_r <= bk.y + brick_h) {

                bk.active = false;
                score += 200;

                       // 30% chance to drop a powerup container pill
                if ((rand() % 10) < 3) {
                    PowerUp p;
                    p.x = bk.x + (brick_w / 2.0f);
                    p.y = bk.y + (brick_h / 2.0f);
                    p.speed = 3.0f;
                    p.active = true;
                    p.type = (PowerUpType)(rand() % 3);
                    falling_bonuses.push_back(p);
                }

                float overlap_left   = (ball_x + ball_r) - bk.x;
                float overlap_right  = (bk.x + brick_w) - (ball_x - ball_r);
                float overlap_top    = (ball_y + ball_r) - bk.y;
                float overlap_bottom = (bk.y + brick_h) - (ball_y - ball_r);

                float min_overlap_h = (overlap_left < overlap_right) ? overlap_left : overlap_right;
                float min_overlap_v = (overlap_top < overlap_bottom) ? overlap_top : overlap_bottom;

                if (min_overlap_h < min_overlap_v) {
                    ball_dx *= -1.0f;
                } else {
                    ball_dy *= -1.0f;
                }
                break;
            }
        }

        if (all_destroyed) victory = true;

               // --- 💊 POWER-UP ACTIVE MODIFIER UPDATES ---
        if (has_active_powerup) {
            powerup_timer -= 16.67f;
            if (powerup_timer <= 0) {
                paddle_w = 120.0f;   // Reset size
                ball_dy = (ball_dy > 0 ? 5.0f : -5.0f); // Reset speed normalization values
                has_active_powerup = false;
            }
        }

               // Falling capsules tracking
        for (auto& p : falling_bonuses) {
            if (!p.active) continue;
            p.y += p.speed;

                   // Catch test
            if (p.y >= paddle_y && p.y <= paddle_y + paddle_h &&
                p.x >= pad_left && p.x <= pad_right) {

                p.active = false;
                has_active_powerup = true;
                powerup_timer = 8000.0f; // Active status modifications live for 8 seconds

                switch (p.type) {
                case SPEED_BOOST:
                    ball_dy = (ball_dy > 0 ? 8.0f : -8.0f);
                    break;
                case EXTRA_LIFE:
                    lives++;
                    break;
                case GROW_PADDLE:
                    paddle_w = 200.0f;
                    break;
                }
            }
            if (p.y > height) p.active = false;
        }

               // Frame Pacing Throttle Pacer
        long sleep_duration_ms = get_smart_throttle_sleep_ms(true);
        if (sleep_duration_ms > 0) {
            usleep(sleep_duration_ms * 1000);
        }

        refresh = true;
    }

    void Render() override {
        VkvgContext ctx = vkvg_create(surf);
        if (!ctx) return;

               // Background space fill
        vkvg_set_source_rgba(ctx, 0.04f, 0.04f, 0.06f, 1.0f);
        vkvg_paint(ctx);

               // Render Bricks
        float brick_w = 90.0f;
        float brick_h = 24.0f;
        for (const auto& bk : bricks) {
            if (!bk.active) continue;

            vkvg_set_source_rgba(ctx, BRICK_COLORS[bk.type][0], BRICK_COLORS[bk.type][1], BRICK_COLORS[bk.type][2], 1.0f);
            vkvg_rectangle(ctx, bk.x, bk.y, brick_w, brick_h);
            vkvg_fill(ctx);

            vkvg_set_source_rgba(ctx, 1.0f, 1.0f, 1.0f, 0.4f);
            vkvg_set_line_width(ctx, 1.5f);
            vkvg_rectangle(ctx, bk.x, bk.y, brick_w, brick_h);
            vkvg_stroke(ctx);
        }

               // Render Paddle Platform
        vkvg_set_source_rgba(ctx, 0.8f, 0.85f, 0.95f, 1.0f);
        vkvg_rectangle(ctx, paddle_x - (paddle_w / 2.0f), paddle_y, paddle_w, paddle_h);
        vkvg_fill(ctx);

        vkvg_set_source_rgba(ctx, 0.4f, 0.7f, 1.0f, 1.0f);
        vkvg_set_line_width(ctx, 2.0f);
        vkvg_rectangle(ctx, paddle_x - (paddle_w / 2.0f), paddle_y, paddle_w, paddle_h);
        vkvg_stroke(ctx);

               // Render Ball Object
        vkvg_arc(ctx, ball_x, ball_y, ball_r, 0, 2.0f * PI);
        vkvg_set_source_rgba(ctx, 1.0f, 0.35f, 0.2f, 1.0f);
        vkvg_fill(ctx);

               // Render Falling Powerup Pills
        for (const auto& p : falling_bonuses) {
            if (!p.active) continue;

            vkvg_save(ctx);

            if (p.type == SPEED_BOOST)       vkvg_set_source_rgba(ctx, 1.0f, 0.2f, 0.2f, 1.0f);
            else if (p.type == EXTRA_LIFE)   vkvg_set_source_rgba(ctx, 0.2f, 1.0f, 0.4f, 1.0f);
            else if (p.type == GROW_PADDLE)  vkvg_set_source_rgba(ctx, 0.2f, 0.6f, 1.0f, 1.0f);

            vkvg_arc(ctx, p.x - 6.0f, p.y, 6.0f, PI/2.0f, 3.0f * PI / 2.0f);
            vkvg_arc(ctx, p.x + 6.0f, p.y, 6.0f, 3.0f * PI / 2.0f, PI / 2.0f);
            vkvg_close_path(ctx);
            vkvg_fill(ctx);

            vkvg_restore(ctx);
        }

               // Status Screen HUD Condition Overlays
        if (game_over) {
            vkvg_set_source_rgba(ctx, 1.0f, 0.1f, 0.1f, 0.7f);
            vkvg_rectangle(ctx, 0, height / 2.0f - 50.0f, width, 100.0f);
            vkvg_fill(ctx);
        } else if (victory) {
            vkvg_set_source_rgba(ctx, 0.1f, 0.9f, 0.3f, 0.7f);
            vkvg_rectangle(ctx, 0, height / 2.0f - 50.0f, width, 100.0f);
            vkvg_fill(ctx);
        }

        vkvg_destroy(ctx);
        refresh = false;
    }

    void OnMouseMove(double xpos, double ypos) override {
        if (game_over || victory) return;

        paddle_x = (float)xpos;
        if (paddle_x < paddle_w / 2.0f) paddle_x = paddle_w / 2.0f;
        if (paddle_x > width - (paddle_w / 2.0f)) paddle_x = width - (paddle_w / 2.0f);
    }

    long get_smart_throttle_sleep_ms(bool sweep_enabled) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        long current_ms = ts.tv_nsec / 1000000;
        long frame_target = 16;
        long sleep_time = frame_target - (current_ms % frame_target);
        return sleep_time > 0 ? sleep_time : 1;
    }
};
int main(int argc, char** argv) {
    ArkanoidApp app{};
    app.Run();
}