#include "VkvgApp.hpp"

#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <chrono>

#define PI 3.14159265358979323846

class SpeedoApp : public VkvgApp {
    // --- Configuration Block ---
    double targetQuantity   = 0.0;    // Target input state value
    double smoothedQuantity = 0.0;    // Damped system position tracker
    double quantityVelocity = 0.0;    // Damped system velocity tracker
    double maxQuantity      = 220.0;  // Gauge ceiling capacity metric

           // Second-Order System Mechanics (Critically Damped Configuration)
           // Adjust 'frequency' for responsiveness (higher = snappier, lower = heavier inertia)
    double frequency        = 4.5;
    double lastFrameTime    = 0.0;    // High-resolution timer index

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

        if (lastFrameTime == 0.0) {
            lastFrameTime = currentTime;
            return; // Protect starting frame initialization spike
        }

        double deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

               // Sub-stepping time accumulator to prevent tracking explosions or warp skips
        if (deltaTime > 0.05) deltaTime = 0.05;

               // --- TESTING SIMULATION PROTOTYPE ---
        targetQuantity = (sin(currentTime * 0.5) * 0.5 + 0.5) * maxQuantity;

               // --- Semi-Implicit Euler Critically Damped Spring Integration ---
               // Provides flawless mechanical weight, soft acceleration, and smooth settling.
        double omega = 2.0 * PI * frequency;
        double springForce = (targetQuantity - smoothedQuantity) * (omega * omega);
        double dampingForce = quantityVelocity * (2.0 * omega); // Zeta = 1.0 (Critical Damping)

        double acceleration = springForce - dampingForce;

        quantityVelocity += acceleration * deltaTime;
        smoothedQuantity += quantityVelocity * deltaTime;

               // Force frame updates dynamically whenever physics parameters are actively moving
        if (fabs(quantityVelocity) > 0.001 || fabs(targetQuantity - smoothedQuantity) > 0.01) {
            refresh = true;
        }
    }

    void Render() override {
        VkvgContext ctx = vkvg_create(surf);

               // Clear Background with a sleek dark slate finish
        vkvg_set_source_rgba(ctx, 0.12f, 0.14f, 0.18f, 1.0f);
        vkvg_paint(ctx);

        vkvg_translate(ctx, offsetX, offsetY);
        vkvg_scale(ctx, zoom, zoom);

               // Gauge Dimensions Configuration
        float cx = 300.0f;
        float cy = 300.0f;
        float radius = 180.0f;

        float startAngle = 0.75f * M_PI;
        float endAngle   = 2.25f * M_PI;
        float totalSweep = endAngle - startAngle;

               // 1. Draw Outer Dial Background Arc Circle Track
        vkvg_set_source_rgba(ctx, 0.2f, 0.24f, 0.3f, 0.4f);
        vkvg_set_line_width(ctx, 8.0f);
        vkvg_arc(ctx, cx, cy, radius, startAngle, endAngle);
        vkvg_stroke(ctx);

               // Clamping the normalized physical position to ensure visual safety limits
        float speedPercentage = (float)(smoothedQuantity / maxQuantity);
        if (speedPercentage > 1.05f) speedPercentage = 1.05f; // Minimal mechanical over-sweep padding
        if (speedPercentage < -0.01f) speedPercentage = -0.01f;

               // 2. Draw Active Quantitative Fill Progress Bar Arc Tracker
               // FIX: Ensure colorPercent can absolutely never drop below 0.0f or exceed 1.0f
        float colorPercent = speedPercentage;
        if (colorPercent > 1.0f) colorPercent = 1.0f;
        if (colorPercent < 0.0f) colorPercent = 0.0f;

               // FIX: Bound currentFillAngle strictly so it cannot drop behind the start pin position
        float currentFillAngle = startAngle + (speedPercentage * totalSweep);
        if (currentFillAngle < startAngle) {
            currentFillAngle = startAngle;
        }

        vkvg_set_source_rgba(ctx, 0.1f + (colorPercent * 0.8f), 0.7f - (colorPercent * 0.5f), 0.9f - (colorPercent * 0.7f), 1.0f);
        vkvg_set_line_width(ctx, 10.0f);

               // FIX: Enforce a strict minimum sweep threshold (0.01 radians).
               // If the angle difference is too small, skip the arc entirely to prevent VKVG geometry shader artifacts.
        if (currentFillAngle - startAngle > 0.01f) {
            vkvg_arc(ctx, cx, cy, radius, startAngle, currentFillAngle);
            vkvg_stroke(ctx);
        }



        vkvg_set_source_rgba(ctx, 0.1f + (colorPercent * 0.8f), 0.7f - (colorPercent * 0.5f), 0.9f - (colorPercent * 0.7f), 1.0f);
        vkvg_set_line_width(ctx, 10.0f);

        // FIX: Only perform the stroke if we have a real distance to fill past the zero pin
        if (currentFillAngle > startAngle + 0.001f) {
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

               // Display Numeric Output Context values
        float baseTextY = cy + 40.0f;

        vkvg_set_font(ctx, font48);
        vkvg_set_source_rgba(ctx, 1.0f, 1.0f, 1.0f, 1.0f);
        char valueStr[32];

        // Safeguard counter value displayed to absolute 0 bounds to avoid numerical micro-noise negatives
        double displayVal = (smoothedQuantity < 0.0) ? 0.0 : smoothedQuantity;
        sprintf(valueStr, "%03.1f", displayVal);

        vkvg_text_extents_t valExtents;
        vkvg_text_extents(ctx, valueStr, &valExtents);
        vkvg_move_to(ctx, cx - valExtents.width / 2.0f, baseTextY + 30.0f);
        vkvg_show_text(ctx, valueStr);

        vkvg_set_font(ctx, font16);
        vkvg_set_source_rgba(ctx, 0.5f, 0.55f, 0.65f, 1.0f);
        const char* unitLabel = "UNITS / SEC";
        vkvg_text_extents(ctx, unitLabel, &valExtents);
        vkvg_move_to(ctx, cx - valExtents.width / 2.0f, baseTextY + 60.0f);
        vkvg_show_text(ctx, unitLabel);

               // 4. Render Physical Needle Shadow Line Offset
        vkvg_set_source_rgba(ctx, 0.05f, 0.05f, 0.05f, 0.4f);
        vkvg_set_line_width(ctx, 4.0f);
        vkvg_move_to(ctx, cx + 4.0f, cy + 4.0f);
        vkvg_line_to(ctx, cx + (radius - 15.0f) * cos(currentFillAngle) + 4.0f, cy + (radius - 15.0f) * sin(currentFillAngle) + 4.0f);
        vkvg_stroke(ctx);

               // 5. Render Core Main Needle Indicator Line Track
        vkvg_set_source_rgba(ctx, 1.0f, 0.23f, 0.23f, 1.0f);
        vkvg_set_line_width(ctx, 4.0f);
        vkvg_move_to(ctx, cx, cy);
        vkvg_line_to(ctx, cx + (radius - 15.0f) * cos(currentFillAngle), cy + (radius - 15.0f) * sin(currentFillAngle));
        vkvg_stroke(ctx);

               // 6. Draw Center Pivot Cap Details
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
