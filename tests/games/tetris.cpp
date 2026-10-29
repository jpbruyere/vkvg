#include "VkvgApp.hpp"

#include <vector>
#include <iostream>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

// Grid configuration metrics
#define GRID_COLS 10
#define GRID_ROWS 20

// Matrix definition tables for all 7 standard pieces (oriented within 4x4 matrix spaces)
const int TETROMINOES[7][4][4] = {
    {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}}, // I Piece
    {{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}}, // J Piece
    {{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}}, // L Piece
    {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}, // O Piece
    {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}}, // S Piece
    {{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}}, // T Piece
    {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}  // Z Piece
};

// Vibrant color slots matched cleanly across the piece index values
const float PIECE_COLORS[8][3] = {
    {0.1f, 0.1f, 0.12f},  // Empty grid gray border
    {0.0f, 0.9f, 0.9f},   // Cyan I
    {0.0f, 0.2f, 0.9f},   // Blue J
    {0.9f, 0.5f, 0.0f},   // Orange L
    {0.9f, 0.9f, 0.0f},   // Yellow O
    {0.0f, 0.9f, 0.0f},   // Green S
    {0.6f, 0.0f, 0.9f},   // Purple T
    {0.9f, 0.0f, 0.0f}    // Red Z
};

class TetrisApp : public VkvgApp {

  public:
    int board[GRID_ROWS][GRID_COLS] = {0};

    // Active falling item states
    int current_piece[4][4] = {0};
    int current_type = 0;
    int piece_x = 3;
    int piece_y = 0;

    float gravity_accumulator = 0.0f;
    float gravity_delay_ms = 500.0f; // Speed threshold of dropping blocks

    int score = 0;
    bool game_over = false;

    void Init() override {
        VkvgApp::Init();
        srand((unsigned int)time(NULL));
        spawn_new_piece();
    }

    void spawn_new_piece() {
        current_type = rand() % 7;
        piece_x = 3;
        piece_y = 0;

        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                current_piece[r][c] = TETROMINOES[current_type][r][c] ? (current_type + 1) : 0;
            }
        }

        // Immediate collision check on spawn coordinates determines game over thresholds
        if (check_collision(piece_x, piece_y, current_piece)) {
            game_over = true;
        }
    }

    bool check_collision(int next_x, int next_y, int piece_mat[4][4]) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (piece_mat[r][c] != 0) {
                    int board_x = next_x + c;
                    int board_y = next_y + r;

                    if (board_x < 0 || board_x >= GRID_COLS || board_y >= GRID_ROWS) {
                        return true; // Wall/Floor bounds hit
                    }
                    if (board_y >= 0 && board[board_y][board_x] != 0) {
                        return true; // Existing locked layout cell overlap
                    }
                }
            }
        }
        return false;
    }

    void lock_piece() {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (current_piece[r][c] != 0) {
                    int board_y = piece_y + r;
                    int board_x = piece_x + c;
                    if (board_y >= 0 && board_y < GRID_ROWS && board_x >= 0 && board_x < GRID_COLS) {
                        board[board_y][board_x] = current_piece[r][c];
                    }
                }
            }
        }
        clear_lines();
        spawn_new_piece();
    }

    void clear_lines() {
        for (int r = GRID_ROWS - 1; r >= 0; r--) {
            bool row_full = true;
            for (int c = 0; c < GRID_COLS; c++) {
                if (board[r][c] == 0) {
                    row_full = false;
                    break;
                }
            }
            if (row_full) {
                score += 100;
                // Collapse rows above downward
                for (int move_r = r; move_r > 0; move_r--) {
                    for (int c = 0; c < GRID_COLS; c++) {
                        board[move_r][c] = board[move_r - 1][c];
                    }
                }
                for (int c = 0; c < GRID_COLS; c++) {
                    board[0][c] = 0;
                }
                r++; // Re-evaluate index level position
            }
        }
    }

    void rotate_piece() {
        // The O-Piece (Type 3) has a symmetric center of mass and never alters orientation
        if (current_type == 3) return;

        int rotated[4][4] = {0};

        // 1. Resolve bounding grid context sizes based on standard piece typography
        // The 'I' piece (Type 0) occupies a full 4x4 matrix space. All others use a 3x3 box.
        int box_size = (current_type == 0) ? 4 : 3;

               // 2. Perform matrix transposition relative strictly to the piece's actual box bounds
        for (int r = 0; r < box_size; r++) {
            for (int c = 0; c < box_size; c++) {
                rotated[c][box_size - 1 - r] = current_piece[r][c];
            }
        }

               // 3. Wall Kick Simulation: Check structural offsets if the rotation hits a boundary edge
               // We test the base rotation, then a 1-block shift left, then a 1-block shift right
        int kick_offsets[] = {0, -1, 1};

        for (int i = 0; i < 3; i++) {
            int test_x = piece_x + kick_offsets[i];
            if (!check_collision(test_x, piece_y, rotated)) {
                // Securely execute rotation state swap once a valid bounding position is found
                piece_x = test_x;
                for (int r = 0; r < 4; r++) {
                    for (int c = 0; c < 4; c++) {
                        current_piece[r][c] = rotated[r][c];
                    }
                }
                return;
            }
        }
    }


    void Update() override {
        if (game_over) return;

               // Force frame tick pacing intervals using 60fps benchmark targets
        long sleep_duration_ms = get_smart_throttle_sleep_ms(true);
        if (sleep_duration_ms > 0) {
            usleep(sleep_duration_ms * 1000);
        }

               // Drop piece step calculation blocks based on elapsed gravity ticks
        gravity_accumulator += 16.67f; // Approx ms elapsed per frame pacing tick
        if (gravity_accumulator >= gravity_delay_ms) {
            gravity_accumulator = 0.0f;
            if (!check_collision(piece_x, piece_y + 1, current_piece)) {
                piece_y++;
            } else {
                lock_piece();
            }
        }

        refresh = true;
    }

    void Render() override {
        VkvgContext ctx = vkvg_create(surf);
        if (!ctx) return;

               // Dark clean arcade retro backspace fill
        vkvg_set_source_rgba(ctx, 0.05f, 0.05f, 0.07f, 1.0f);
        vkvg_paint(ctx);

               // Center matrix coordinates calculation to track layout scale bounds nicely
        float block_size = 36.0f;
        float board_w = GRID_COLS * block_size;
        float board_h = GRID_ROWS * block_size;
        float start_x = (width - board_w) / 2.0f;
        float start_y = (height - board_h) / 2.0f;

               // --- 🧱 DRAW LOCKED BACKGROUND MATRIX LAYOUT ---
        for (int r = 0; r < GRID_ROWS; r++) {
            for (int c = 0; c < GRID_COLS; c++) {
                int color_idx = board[r][c];
                vkvg_set_source_rgba(ctx, PIECE_COLORS[color_idx][0], PIECE_COLORS[color_idx][1], PIECE_COLORS[color_idx][2], 1.0f);
                vkvg_rectangle(ctx, start_x + c * block_size, start_y + r * block_size, block_size - 2.0f, block_size - 2.0f);
                vkvg_fill(ctx);
            }
        }

               // --- 🧊 DRAW CURRENT ACTIVE FALLING PIECE ---
        if (!game_over) {
            for (int r = 0; r < 4; r++) {
                for (int c = 0; c < 4; c++) {
                    if (current_piece[r][c] != 0) {
                        int color_idx = current_type + 1;
                        vkvg_set_source_rgba(ctx, PIECE_COLORS[color_idx][0], PIECE_COLORS[color_idx][1], PIECE_COLORS[color_idx][2], 1.0f);
                        vkvg_rectangle(ctx, start_x + (piece_x + c) * block_size, start_y + (piece_y + r) * block_size, block_size - 2.0f, block_size - 2.0f);
                        vkvg_fill(ctx);
                    }
                }
            }
        }

               // --- 🚨 GAME OVER DISPLAY OVERLAY ---
        if (game_over) {
            vkvg_set_source_rgba(ctx, 1.0f, 0.1f, 0.1f, 0.7f);
            vkvg_rectangle(ctx, start_x, height / 2.0f - 40.0f, board_w, 80.0f);
            vkvg_fill(ctx);
        }

        vkvg_destroy(ctx);
        refresh = false;
    }

           // --- 🕹️ INTERACTIVE GLFW INPUT OVERRIDES ---
    void OnKey(int key, int scancode, int action, int mods) override {
        if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
        if (game_over) return;

        switch (key) {
        case GLFW_KEY_LEFT:
            if (!check_collision(piece_x - 1, piece_y, current_piece)) {
                piece_x--;
            }
            break;
        case GLFW_KEY_RIGHT:
            if (!check_collision(piece_x + 1, piece_y, current_piece)) {
                piece_x++;
            }
            break;
        case GLFW_KEY_DOWN:
            if (!check_collision(piece_x, piece_y + 1, current_piece)) {
                piece_y++;
            }
            break;
        case GLFW_KEY_UP:
            rotate_piece();
            break;
        case GLFW_KEY_SPACE:
            // Hard Drop piece straight down to locked position
            while (!check_collision(piece_x, piece_y + 1, current_piece)) {
                piece_y++;
            }
            lock_piece();
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(win, GLFW_TRUE);
            break;
        }
        refresh = true; // Trigger instant redraw on key input
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
    TetrisApp app{};
    app.Run();
}
