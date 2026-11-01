#include "VkvgApp.hpp"

#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <chrono>
#include <string>

class OdometerApp : public VkvgApp {
    double targetQuantity   = 0.0;
    double smoothedQuantity = 0.0;
    double lastFrameTime    = 0.0;
    double throttleSpeed    = 0.002;    // Balanced snappy interpolation for fluid response

    VkvgFont odometerFont = NULL;

    void Init() override {
        VkvgApp::Init();
        odometerFont = vkvg_font_create(device, "roboto:bold", 46);
    }

    void CleanUp() override {
        vkvg_font_destroy(odometerFont);
        VkvgApp::CleanUp();
    }

    void Update() override {
        double currentTime = 0.0;
        if (!displayMode) {
            currentTime = glfwGetTime();
        } else {
            auto now = std::chrono::high_resolution_clock::now();
            auto duration = now.time_since_epoch();
            currentTime = std::chrono::duration<double>(duration).count();
        }

               // Fix the weird startup jumping behavior by anchoring the baseline instantly
        if (lastFrameTime == 0.0) {
            lastFrameTime = currentTime;
            return; // Skip the first frame to avoid a massive deltaTime spike
        }

        double deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

               // Strict clamp for execution spikes (e.g. windows dragging or focus changes)
        if (deltaTime > 0.1) deltaTime = 0.1;

               // Smoothly interpolate the rolling value using frame-rate independent physics
        double interpolationFactor = 1.0 - exp(-throttleSpeed * deltaTime);
        smoothedQuantity += (targetQuantity - smoothedQuantity) * interpolationFactor;

               // Incremental accumulation of quantity over time
        targetQuantity += deltaTime * 2.5;

               // CRITICAL FIX: Force continuous Vulkan updates every single loop iteration.
               // This stops vkh_presenter_draw from stalling out of sync with state data.
        refresh = true;
    }


    void DrawDrumColumn(VkvgContext ctx, float x, float y, float w, float h, double continuousValue, bool isRedDrum) {
        vkvg_save(ctx);

        // 1. Clipping region for the narrow horizontal wheel slot
        vkvg_rectangle(ctx, x, y, w, h);
        vkvg_clip(ctx);

               // 2. Base track color fill
        if (isRedDrum) {
            vkvg_set_source_rgba(ctx, 0.72f, 0.08f, 0.08f, 1.0f);
        } else {
            vkvg_set_source_rgba(ctx, 0.94f, 0.92f, 0.88f, 1.0f);
        }
        vkvg_rectangle(ctx, x, y, w, h);
        vkvg_fill(ctx);

               // 3. Extract the clean mathematical fractional rotation components
        double value = fmod(continuousValue, 10.0);
        if (value < 0) value += 10.0;

        double fractionalTurn = modf(value, &continuousValue);
        int currentDigit = ((int)continuousValue) % 10;
        int nextDigit = (currentDigit + 1) % 10;

               // 4. Text configurations
        vkvg_set_font(ctx, odometerFont);
        if (isRedDrum) {
            vkvg_set_source_rgba(ctx, 0.98f, 0.98f, 0.98f, 1.0f);
        } else {
            vkvg_set_source_rgba(ctx, 0.08f, 0.08f, 0.08f, 1.0f);
        }

               // CRITICAL FIX 1: Font spacing is now locked to the height of the drum window.
               // This eliminates the empty space layout gap.
        float fontSpacing = h;

        // Center alignment targeting the perfect middle of the drum
        float centerY = y + h / 2.0f + 15.0f;

               // Render standard current digit tracking down smoothly
        char buf[16];
        sprintf(buf, "%d", currentDigit);
        vkvg_text_extents_t ext;
        vkvg_text_extents(ctx, buf, &ext);
        vkvg_move_to(ctx, x + (w - ext.width) / 2.0f, centerY - (fractionalTurn * fontSpacing));
        vkvg_show_text(ctx, buf);

               // Render subsequent trailing digit tracking cleanly right behind it
        sprintf(buf, "%d", nextDigit);
        vkvg_text_extents(ctx, buf, &ext);
        vkvg_move_to(ctx, x + (w - ext.width) / 2.0f, centerY + fontSpacing - (fractionalTurn * fontSpacing));
        vkvg_show_text(ctx, buf);

        vkvg_restore(ctx);

               // 5. Deeper 3D Linear gradient overlay to compress the numbers visually near the edges
        VkvgPattern shadowPattern = vkvg_pattern_create_linear(x, y, x, y + h);
        vkvg_pattern_add_color_stop(shadowPattern, 0.0f,  0.0f, 0.0f, 0.0f, 0.92f); // Heavy top edge curve shadow
        vkvg_pattern_add_color_stop(shadowPattern, 0.20f, 0.0f, 0.0f, 0.0f, 0.20f);
        vkvg_pattern_add_color_stop(shadowPattern, 0.5f,  0.0f, 0.0f, 0.0f, 0.0f);  // Flat bright face apex
        vkvg_pattern_add_color_stop(shadowPattern, 0.80f, 0.0f, 0.0f, 0.0f, 0.20f);
        vkvg_pattern_add_color_stop(shadowPattern, 1.0f,  0.0f, 0.0f, 0.0f, 0.92f); // Heavy bottom edge curve shadow

        vkvg_set_source(ctx, shadowPattern);
        vkvg_rectangle(ctx, x, y, w, h);
        vkvg_fill(ctx);
        vkvg_pattern_destroy(shadowPattern);
    }


    void Render() override {
        VkvgContext ctx = vkvg_create(surf);

               // Dark matte dashboard casing texture background
        vkvg_set_source_rgba(ctx, 0.09f, 0.10f, 0.12f, 1.0f);
        vkvg_paint(ctx);

        vkvg_translate(ctx, offsetX, offsetY);
        vkvg_scale(ctx, zoom, zoom);

        float startX = 200.0f;
        float startY = 300.0f;
        float drumWidth = 42.0f;
        float drumHeight = 52.0f;
        float gap = 2.0f;

               // Raw un-rounded target metrics
        double totalValue = smoothedQuantity;

               // PURE LINEAR MECHANICAL CASCADE
               // No floor steps or conditionals. Drums move completely proportionally at all times.
        double hund = totalValue * 100.0;
        double tent = totalValue * 10.0;
        double ones = totalValue;
        double tens = totalValue / 10.0;
        double hnds = totalValue / 100.0;

               // Render white drums
        DrawDrumColumn(ctx, startX + 0 * (drumWidth + gap), startY, drumWidth, drumHeight, hnds, false);
        DrawDrumColumn(ctx, startX + 1 * (drumWidth + gap), startY, drumWidth, drumHeight, tens, false);
        DrawDrumColumn(ctx, startX + 2 * (drumWidth + gap), startY, drumWidth, drumHeight, ones, false);

               // Tiny divider split accent
        vkvg_set_source_rgba(ctx, 0.15f, 0.15f, 0.15f, 1.0f);
        vkvg_rectangle(ctx, startX + 3 * (drumWidth + gap) - 1.0f, startY, 1.0f, drumHeight);
        vkvg_fill(ctx);

               // Render last two numbers on red cylinders
        DrawDrumColumn(ctx, startX + 3 * (drumWidth + gap), startY, drumWidth, drumHeight, tent, true);
        DrawDrumColumn(ctx, startX + 4 * (drumWidth + gap), startY, drumWidth, drumHeight, hund, true);

               // Outer Bezel Frame Housing Cover
        vkvg_set_line_width(ctx, 5.0f);
        vkvg_set_source_rgba(ctx, 0.25f, 0.27f, 0.30f, 1.0f);
        vkvg_rectangle(ctx, startX - 5.0f, startY - 5.0f, 5 * (drumWidth + gap) + 6.0f, drumHeight + 10.0f);
        vkvg_stroke(ctx);

        vkvg_destroy(ctx);
        refresh = false;
    }
};

int main(int argc, char** argv) {
    OdometerApp app{};
    app.Run();
}
