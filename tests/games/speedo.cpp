#include "VkvgApp.hpp"

#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <chrono>

#define PI 3.14159265358979323846

class SpeedoApp : public VkvgApp {
    // Add these variables inside the public or protected block of your VkvgApp class
    double targetQuantity   = 0.0;    // The absolute current value (e.g. from an engine/sensor)
    double smoothedQuantity = 0.0;    // The smoothly interpolated value used for rendering
    double maxQuantity      = 220.0;  // Max capacity value on your gauge dial

    // Multi-platform high-resolution timing metrics
    double lastFrameTime    = 0.0;    // High-resolution timestamp marker
    double throttleSpeed    = 8.0;    // Interpolation smoothing weight (higher = snappier)

    VkvgFont font14 = NULL;
    VkvgFont font16 = NULL;
    VkvgFont font48 = NULL;

    void Init() override {
        VkvgApp::Init();
        font14 = vkvg_font_create (device, "sans-serif", 14);
        font16 = vkvg_font_create (device, "sans-serif", 16);
        font48 = vkvg_font_create (device, "sans-serif", 48);
    }
    void CleanUp() override {
        vkvg_font_destroy(font14);
        vkvg_font_destroy(font16);
        vkvg_font_destroy(font48);
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

               // Protect initialization frame spike
        if (lastFrameTime == 0.0) {
            lastFrameTime = currentTime;
        }

        double deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

               // Clamping large delta bounds (e.g. background application pausing or window dragging)
        if (deltaTime > 0.1) deltaTime = 0.1;

               // Multi-platform Smooth Throttling (Exponential Delta Interpolation)
               // Formula: smoothed = smoothed + (target - smoothed) * (1 - e^(-speed * dt))
        double interpolationFactor = 1.0 - exp(-throttleSpeed * deltaTime);
        double previousValue = smoothedQuantity;

        smoothedQuantity += (targetQuantity - smoothedQuantity) * interpolationFactor;

               // Trigger frame updates only if the metric moved past a visual pixel threshold
        if (fabs(smoothedQuantity - previousValue) > 0.01) {
            refresh = true;
        }

        // --- TESTING SIMULATION PROTOTYPE ---
        // Comment this out when linking real asset streams.
        // Oscillates the target value cleanly back and forth to test visual behaviors.
        targetQuantity = (sin(currentTime * 0.5) * 0.5 + 0.5) * maxQuantity;
    }

    void Render() override {
        VkvgContext ctx = vkvg_create(surf);

               // Clear Background with a sleek dark slate finish
        vkvg_set_source_rgba(ctx, 0.12f, 0.14f, 0.18f, 1.0f);
        vkvg_paint(ctx);

        vkvg_translate(ctx, offsetX, offsetY);
        vkvg_scale(ctx, zoom, zoom);

               // Gauge Dimensions Configuration
        float cx = 300.0f;          // Gauge center X
        float cy = 300.0f;          // Gauge center Y
        float radius = 180.0f;      // Outer dial sweep track radius

        // Angles in radians (Vulkan/VKVG system coordinates mirror: 0 = Right, PI/2 = Down)
        float startAngle = 0.75f * M_PI;  // Bottom-Left sweep point
        float endAngle   = 2.25f * M_PI;  // Bottom-Right sweep point
        float totalSweep = endAngle - startAngle;

               // 1. Draw Outer Dial Background Arc Circle Track
        vkvg_set_source_rgba(ctx, 0.2f, 0.24f, 0.3f, 0.4f);
        vkvg_set_line_width(ctx, 8.0f);
        vkvg_arc(ctx, cx, cy, radius, startAngle, endAngle);
        vkvg_stroke(ctx);

               // 2. Draw Active Quantitative Fill Progress Bar Arc Tracker
        float speedPercentage = (float)(smoothedQuantity / maxQuantity);
        if (speedPercentage > 1.0f) speedPercentage = 1.0f;
        if (speedPercentage < 0.0f) speedPercentage = 0.0f;

        float currentFillAngle = startAngle + (speedPercentage * totalSweep);

        // Dynamic color shifting (Cyan when chilling, Red when pushed near limits)
        vkvg_set_source_rgba(ctx, 0.1f + (speedPercentage * 0.8f), 0.7f - (speedPercentage * 0.5f), 0.9f - (speedPercentage * 0.7f), 1.0f);
        vkvg_set_line_width(ctx, 10.0f);
        if (speedPercentage > 0.001f) {
            vkvg_arc(ctx, cx, cy, radius, startAngle, currentFillAngle);
            vkvg_stroke(ctx);
        }

               // 3. Render Circular Tick Marks and Numbers around the Face
        int numTicks = 11;
        vkvg_set_line_width(ctx, 2.0f);

        vkvg_set_font(ctx, font14);

        for (int i = 0; i < numTicks; i++) {
            float t = (float)i / (numTicks - 1);
            float tickAngle = startAngle + (t * totalSweep);

            float cosA = cos(tickAngle);
            float sinA = sin(tickAngle);

            // Inner and outer tick vertex extensions
            float xOuter = cx + radius * cosA;
            float yOuter = cy + radius * sinA;
            float xInner = cx + (radius - 12.0f) * cosA;
            float yInner = cy + (radius - 12.0f) * sinA;

            vkvg_set_source_rgba(ctx, 0.6f, 0.65f, 0.75f, 0.8f);
            vkvg_move_to(ctx, xInner, yInner);
            vkvg_line_to(ctx, xOuter, yOuter);
            vkvg_stroke(ctx);

            float xText = cx + (radius - 30.0f) * cosA;
            float yText = cy + (radius - 30.0f) * sinA;

            char tickLabel[16];
            sprintf(tickLabel, "%d", (int)(t * maxQuantity));

            vkvg_text_extents_t extents;
            vkvg_text_extents(ctx, tickLabel, &extents);

            vkvg_move_to(ctx, xText - extents.width / 2.0f, yText + extents.height / 2.0f);
            vkvg_show_text(ctx, tickLabel);
        }

        cy += 40;

        vkvg_set_font(ctx, font48);
        vkvg_set_source_rgba(ctx, 1.0f, 1.0f, 1.0f, 1.0f);
        char valueStr[32];
        sprintf(valueStr, "%03.1f", smoothedQuantity);

        vkvg_text_extents_t valExtents;
        vkvg_text_extents(ctx, valueStr, &valExtents);
        vkvg_move_to(ctx, cx - valExtents.width / 2.0f, cy + 30.0f);
        vkvg_show_text(ctx, valueStr);

        vkvg_set_font(ctx, font16);
        vkvg_set_source_rgba(ctx, 0.5f, 0.55f, 0.65f, 1.0f);
        const char* unitLabel = "UNITS / SEC";
        vkvg_text_extents(ctx, unitLabel, &valExtents);
        vkvg_move_to(ctx, cx - valExtents.width / 2.0f, cy + 60.0f);
        vkvg_show_text(ctx, unitLabel);

        cy -= 40;

        vkvg_set_source_rgba(ctx, 0.05f, 0.05f, 0.05f, 0.4f);
        vkvg_set_line_width(ctx, 4.0f);
        vkvg_move_to(ctx, cx + 4.0f, cy + 4.0f);
        vkvg_line_to(ctx, cx + (radius - 15.0f) * cos(currentFillAngle) + 4.0f, cy + (radius - 15.0f) * sin(currentFillAngle) + 4.0f);
        vkvg_stroke(ctx);

        vkvg_set_source_rgba(ctx, 1.0f, 0.23f, 0.23f, 1.0f);
        vkvg_set_line_width(ctx, 4.0f);
        vkvg_move_to(ctx, cx, cy);
        vkvg_line_to(ctx, cx + (radius - 15.0f) * cos(currentFillAngle), cy + (radius - 15.0f) * sin(currentFillAngle));
        vkvg_stroke(ctx);

        vkvg_arc(ctx, cx, cy, 10.0f, 0, 2.0f * M_PI);
        vkvg_set_source_rgba(ctx, 0.15f, 0.18f, 0.22f, 1.0f);
        vkvg_fill(ctx);
        vkvg_arc(ctx, cx, cy, 10.0f, 0, 2.0f * M_PI);
        vkvg_set_source_rgba(ctx, 1.0f, 0.23f, 0.23f, 1.0f);
        vkvg_set_line_width(ctx, 2.0f);
        vkvg_stroke(ctx);

        vkvg_destroy(ctx);
        refresh = false;
    }
};

int main(int argc, char** argv) {
    SpeedoApp app{};
    app.Run();
}
