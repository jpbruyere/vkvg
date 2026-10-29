#include "VkvgApp.hpp"

#include <vector>
#include <iostream>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

// Vector structures for game entities
struct Laser {
    float x, y;
    bool active;
};

struct Invader {
    float x, y;
    bool alive;
    int type; // Different visual profiles
};

class SpaceInvadersApp : public VkvgApp {

  public:
    // Game state tracking
    float player_x = 512.0f;
    float player_y = 900.0f;
    float player_w = 60.0f;
    float player_h = 20.0f;

    std::vector<Laser> lasers;
    std::vector<Invader> invaders;

    float invader_speed = 1.5f;
    int invader_direction = 1; // 1 = Right, -1 = Left
    float invader_drop_timer = 0.0f;

    int score = 0;
    bool game_over = false;

    void Init() override {
        VkvgApp::Init();

        // Center player initially based on resolved window width
        player_x = width / 2.0f;
        player_y = height * 0.88f;

               // Spawn a classic 5x8 grid of invaders
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 8; col++) {
                Invader ali;
                ali.x = 150.0f + col * 80.0f;
                ali.y = 100.0f + row * 60.0f;
                ali.alive = true;
                ali.type = row % 2;
                invaders.push_back(ali);
            }
        }
    }

    void Update() override {
        if (game_over) return;

               // --- 🚀 1. UPDATE LASERS ---
        for (size_t i = 0; i < lasers.size(); i++) {
            if (lasers[i].active) {
                lasers[i].y -= 12.0f; // Laser speed
                if (lasers[i].y < 0) {
                    lasers[i].active = false;
                }
            }
        }

               // --- 👾 2. UPDATE INVADERS (GRID MOVEMENT) ---
        bool shift_down = false;
        for (const auto& ali : invaders) {
            if (!ali.alive) continue;

            // Check edge boundaries to switch directions
            if (invader_direction == 1 && ali.x + 40.0f > width - 50.0f) {
                shift_down = true;
            } else if (invader_direction == -1 && ali.x < 50.0f) {
                shift_down = true;
            }
        }

        if (shift_down) {
            invader_direction *= -1;
            for (auto& ali : invaders) {
                if (ali.alive) {
                    ali.y += 20.0f; // Step closer to player
                    if (ali.y + 30.0f >= player_y) {
                        game_over = true; // Invaders reached base
                    }
                }
            }
        } else {
            for (auto& ali : invaders) {
                if (ali.alive) {
                    ali.x += invader_speed * invader_direction;
                }
            }
        }

               // --- 💥 3. COLLISION DETECTION ---
        for (auto& laser : lasers) {
            if (!laser.active) continue;

            for (auto& ali : invaders) {
                if (!ali.alive) continue;

                       // AABB bounding box collision test
                if (laser.x >= ali.x && laser.x <= ali.x + 40.0f &&
                    laser.y >= ali.y && laser.y <= ali.y + 30.0f) {
                    ali.alive = false;
                    laser.active = false;
                    score += 100;
                    break;
                }
            }
        }

               // --- 🧠 PACING THROTTLE (Locked clean at display refresh rate) ---
        long sleep_duration_ms = get_smart_throttle_sleep_ms(true);
        if (sleep_duration_ms > 0) {
            usleep(sleep_duration_ms * 1000);
        }

        refresh = true; // Keep window ticking
    }

    void Render() override {
        VkvgContext ctx = vkvg_create(surf);
        if (!ctx) return;

               // Clear display canvas with an arcade retro black space fill
        vkvg_set_source_rgba(ctx, 0.03f, 0.03f, 0.05f, 1.0f);
        vkvg_paint(ctx);

               // --- 🚀 DRAW DEFENSE DEFENDER ---
        vkvg_set_source_rgba(ctx, 0.0f, 1.0f, 0.33f, 1.0f); // Retro Green Tank
        // Draw main chassis
        vkvg_rectangle(ctx, player_x - (player_w / 2.0f), player_y, player_w, player_h);
        vkvg_fill(ctx);
        // Draw top turret block
        vkvg_rectangle(ctx, player_x - 6.0f, player_y - 8.0f, 12.0f, 8.0f);
        vkvg_fill(ctx);

               // --- ⚡ DRAW ACTIVE INTERCEPT LASERS ---
        vkvg_set_source_rgba(ctx, 1.0f, 0.2f, 0.2f, 1.0f); // Bright Neon Red Lasers
        for (const auto& laser : lasers) {
            if (laser.active) {
                vkvg_rectangle(ctx, laser.x - 2.0f, laser.y, 4.0f, 15.0f);
                vkvg_fill(ctx);
            }
        }

               // --- 👾 DRAW PROCEDURAL ALIEN ENEMY SPRITES ---
        for (const auto& ali : invaders) {
            if (!ali.alive) continue;

            vkvg_save(ctx);
            vkvg_translate(ctx, ali.x, ali.y);

            if (ali.type == 0) {
                vkvg_set_source_rgba(ctx, 1.0f, 0.3f, 0.8f, 1.0f); // Magenta Crabs
                // Visual matrix block setup using clean vector primitives
                vkvg_rectangle(ctx, 5, 5, 30, 10);
                vkvg_rectangle(ctx, 10, 0, 20, 20);
                vkvg_rectangle(ctx, 0, 15, 8, 10);
                vkvg_rectangle(ctx, 32, 15, 8, 10);
            } else {
                vkvg_set_source_rgba(ctx, 0.2f, 0.7f, 1.0f, 1.0f); // Cyan Octopuses
                vkvg_rectangle(ctx, 0, 5, 40, 15);
                vkvg_rectangle(ctx, 8, 0, 24, 25);
                vkvg_rectangle(ctx, 4, 20, 6, 8);
                vkvg_rectangle(ctx, 30, 20, 6, 8);
            }
            vkvg_fill(ctx);

            // Cut out glowing blank pixel eyes using canvas overrides
            vkvg_set_source_rgba(ctx, 0.03f, 0.03f, 0.05f, 1.0f);
            vkvg_rectangle(ctx, 12, 6, 4, 4);
            vkvg_rectangle(ctx, 24, 6, 4, 4);
            vkvg_fill(ctx);

            vkvg_restore(ctx);
        }

               // --- 🏆 SCREEN TEXT HUD DISPLAY ---
               // Render score markers to screen if desired using your registered fonts
               // (Leaving HUD text optional depending on loaded system strings)

        if (game_over) {
            vkvg_set_source_rgba(ctx, 1.0f, 0.1f, 0.1f, 0.8f);
            vkvg_rectangle(ctx, 0, height / 2.0f - 50.0f, width, 100.0f);
            vkvg_fill(ctx);
        }

        vkvg_destroy(ctx);
        refresh = false;
    }

           // --- 🕹️ INTERACTIVE GLFW INPUT OVERRIDES ---
    void OnMouseMove(double xpos, double ypos) override {
        // Map the mouse horizontal axis directly to the ship center position coordinates
        if (!game_over) {
            player_x = (float)xpos;
            // Clamp position securely inside window viewport limits
            if (player_x < player_w / 2.0f) player_x = player_w / 2.0f;
            if (player_x > width - (player_w / 2.0f)) player_x = width - (player_w / 2.0f);
        }
    }

    void OnMouseButton(int button, int action, int mods) override {
        // Tap left-click to unleash a new laser beam element
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !game_over) {
            Laser pulse;
            pulse.x = player_x;
            pulse.y = player_y - 10.0f;
            pulse.active = true;
            lasers.push_back(pulse);

            // Optional: Caps maximum concurrent stray laser allocations to clean memory footprint
            if (lasers.size() > 20) {
                lasers.erase(lasers.begin());
            }
        }
    }

    long get_smart_throttle_sleep_ms(bool sweep_enabled) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        long current_ms = ts.tv_nsec / 1000000;
        long frame_target = 16; // 60 FPS target gameplay pacing
        long sleep_time = frame_target - (current_ms % frame_target);
        return sleep_time > 0 ? sleep_time : 1;
    }
};

int main(int argc, char** argv) {
    SpaceInvadersApp app{};
    app.Run();
}
