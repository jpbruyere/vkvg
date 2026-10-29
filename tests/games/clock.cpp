#include "VkvgApp.hpp"

#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

#define PI 3.14159265358979323846

class ClockApp : public VkvgApp {

  public:
    VkvgFont font;

    void Init ()  override {
        VkvgApp::Init();
        font = vkvg_font_create(device, "sans:bold", 20);
    }

    void CleanUp () override {
        vkvg_font_destroy(font);
        VkvgApp::CleanUp();
    }

    struct tm timeinfo_struct;
    struct tm* timeinfo = &timeinfo_struct;
    float ms;
    bool smooth_sweep = true;

    void Update() override {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);

        struct tm* local_time = localtime(&ts.tv_sec);
        if (local_time) {
            timeinfo_struct = *local_time;
        }

        ms = ts.tv_nsec / 1000000.0f;
        refresh = true;

        long sleep_duration_ms = get_smart_throttle_sleep_ms(smooth_sweep);
        if (sleep_duration_ms > 0) {
            usleep(sleep_duration_ms * 1000);
        }
    }

    void Render() override {
        VkvgContext ctx = vkvg_create(surf);
        if (!ctx) return;

        float cx = width / 2.0f;
        float cy = height / 2.0f;
        float r = (cx < cy ? cx : cy) * 0.85f;

        VkvgPattern bg_grad = vkvg_pattern_create_linear(0, 0, 0, height);
        vkvg_pattern_add_color_stop(bg_grad, 0.0f, 0.15f, 0.17f, 0.20f, 1.0f);
        vkvg_pattern_add_color_stop(bg_grad, 1.0f, 0.08f, 0.09f, 0.10f, 1.0f);
        vkvg_set_source(ctx, bg_grad);
        vkvg_paint(ctx);
        vkvg_pattern_destroy(bg_grad);

        vkvg_set_line_width(ctx, 6.0f);
        vkvg_set_source_rgba(ctx, 0.8f, 0.85f, 0.9f, 0.2f);
        vkvg_arc(ctx, cx, cy, r, 0, 2 * PI);
        vkvg_stroke(ctx);

        vkvg_set_source_rgba(ctx, 0.9f, 0.95f, 1.0f, 0.6f);
        for (int i = 0; i < 12; i++) {
            float angle = i * (PI / 6.0f);
            float inner_r = (i % 3 == 0) ? r * 0.88f : r * 0.93f;
            vkvg_set_line_width(ctx, (i % 3 == 0) ? 4.0f : 2.0f);

            vkvg_move_to(ctx, cx + inner_r * sin(angle), cy - inner_r * cos(angle));
            vkvg_line_to(ctx, cx + r * 0.97f * sin(angle), cy - r * 0.97f * cos(angle));
            vkvg_stroke(ctx);
        }

        float sec_val = timeinfo->tm_sec;
        if (smooth_sweep) {
            sec_val += ms / 1000.0f;
        }
        float min_val = timeinfo->tm_min + sec_val / 60.0f;
        float hour_val = (timeinfo->tm_hour % 12) + min_val / 60.0f;

        float angle_h = hour_val * (PI / 6.0f);
        float angle_m = min_val * (PI / 30.0f);
        float angle_s = sec_val * (PI / 30.0f);

        // --- ⚙️ DRAW INTERLOCKING GEAR TRAIN (Layered Behind Hands) ---
        vkvg_set_source_rgba(ctx, 0.75f, 0.60f, 0.35f, 0.25f); // Frosted Luxury Gold
        draw_gear(ctx, cx, cy, r * 0.45f, 32, angle_m);

        float g2_x = cx + r * 0.32f;
        float g2_y = cy - r * 0.32f;
        vkvg_set_source_rgba(ctx, 0.65f, 0.68f, 0.72f, 0.30f); // Gunmetal Silver
        draw_gear(ctx, g2_x, g2_y, r * 0.22f, 16, -angle_s * 2.0f);

        float g3_x = cx - r * 0.28f;
        float g3_y = cy + r * 0.28f;
        vkvg_set_source_rgba(ctx, 0.85f, 0.40f, 0.30f, 0.15f); // Copper tone
        // Oscillates rapidly simulating a structural tourbillon impulse pulse
        float balance_oscillation = sin(sec_val * PI * 2.5f) * 0.4f;
        draw_gear(ctx, g3_x, g3_y, r * 0.28f, 20, balance_oscillation);

        //draw_day_night_complication(ctx, cx, cy, r);
        //draw_date_window(ctx, cx, cy, r);

        vkvg_set_source_rgba(ctx, 1.0f, 1.0f, 1.0f, 0.9f);
        vkvg_set_line_width(ctx, 8.0f);
        vkvg_set_line_cap(ctx, VKVG_LINE_CAP_ROUND);
        vkvg_move_to(ctx, cx - r * 0.1f * sin(angle_h), cy + r * 0.1f * cos(angle_h));
        vkvg_line_to(ctx, cx + r * 0.5f * sin(angle_h), cy - r * 0.5f * cos(angle_h));
        vkvg_stroke(ctx);

        vkvg_set_line_width(ctx, 5.0f);
        vkvg_move_to(ctx, cx - r * 0.15f * sin(angle_m), cy + r * 0.15f * cos(angle_m));
        vkvg_line_to(ctx, cx + r * 0.75f * sin(angle_m), cy - r * 0.75f * cos(angle_m));
        vkvg_stroke(ctx);

        vkvg_set_source_rgba(ctx, 1.0f, 0.35f, 0.2f, 1.0f);
        vkvg_set_line_width(ctx, 2.0f);
        vkvg_move_to(ctx, cx - r * 0.2f * sin(angle_s), cy + r * 0.2f * cos(angle_s));
        vkvg_line_to(ctx, cx + r * 0.85f * sin(angle_s), cy - r * 0.85f * cos(angle_s));
        vkvg_stroke(ctx);

        vkvg_arc(ctx, cx, cy, 6.0f, 0, 2 * PI);
        vkvg_set_source_rgba(ctx, 1.0f, 0.35f, 0.2f, 1.0f);
        vkvg_fill(ctx);

        vkvg_arc(ctx, cx, cy, 2.0f, 0, 2 * PI);
        vkvg_set_source_rgba(ctx, 1.0f, 1.0f, 1.0f, 1.0f);
        vkvg_fill(ctx);

        vkvg_destroy(ctx);

        refresh = false;
    }
  private:
  private:
    // --- ☀️🌙 POETIC LUXURY CELESTIAL APERTURE ---
    void draw_day_night_complication(VkvgContext ctx, float cx, float cy, float r) {
        float comp_w = r * 0.46f;
        float comp_h = r * 0.26f;
        float comp_x = cx - (comp_w / 2.0f);
        float comp_y = cy - r * 0.86f;

        float disk_cx = cx;
        float disk_cy = comp_y + comp_h;

        vkvg_save(ctx);

               // Clean Horizon Window Mask
        vkvg_arc(ctx, disk_cx, disk_cy, comp_w * 0.5f, PI, 2.0f * PI);
        vkvg_close_path(ctx);
        vkvg_clip(ctx);

        // --- ⚙️ TRUE 24-HOUR GEAR CALIBRATION ---
        // Convert the current time into total elapsed seconds across a true 24-hour period
        float day_seconds = (timeinfo->tm_hour * 3600.0f) + (timeinfo->tm_min * 60.0f) + timeinfo->tm_sec;
        float total_24h_seconds = 86400.0f; // 24 hours * 60 mins * 60 secs

        // Pace the plate rotation smoothly at exactly 1/24th speed of the hour hands.
        // We apply a negative sign so the stars rise on the right and descend on the left.
        float plate_rotation = -(day_seconds / total_24h_seconds) * (2.0f * PI) + (PI / 2.0f);

        vkvg_translate(ctx, disk_cx, disk_cy);
        vkvg_rotate(ctx, plate_rotation);

        float disk_r = (comp_w * 0.5f) * 0.98f;

               // --- THE FULL ROUND CELESTIAL CANVAS ---
               // Instead of splitting the circle with hard lines, we use a beautiful, deep continuous sky blend
        VkvgPattern sky_grad = vkvg_pattern_create_linear(0, -disk_r, 0, disk_r);
        vkvg_pattern_add_color_stop(sky_grad, 0.0f, 0.01f, 0.02f, 0.06f, 1.0f);  // Midnight core
        vkvg_pattern_add_color_stop(sky_grad, 0.4f, 0.05f, 0.12f, 0.25f, 1.0f);  // Soft twilight transition
        vkvg_pattern_add_color_stop(sky_grad, 0.6f, 0.15f, 0.40f, 0.70f, 1.0f);  // Morning glow
        vkvg_pattern_add_color_stop(sky_grad, 1.0f, 0.35f, 0.65f, 0.88f, 1.0f);  // Brilliant midday azure
        vkvg_set_source(ctx, sky_grad);
        vkvg_arc(ctx, 0, 0, disk_r, 0, 2.0f * PI);
        vkvg_fill(ctx);
        vkvg_pattern_destroy(sky_grad);

               // --- ☀️ THE BLAZING SUN (Day Side) ---
        float sun_y = disk_r * 0.58f;
        float sun_radius = disk_r * 0.20f;

        vkvg_save(ctx);
        vkvg_translate(ctx, 0, sun_y);
        vkvg_set_source_rgba(ctx, 1.0f, 0.75f, 0.20f, 0.95f); // Rich Liquid Gold

        for (int i = 0; i < 12; i++) {
            vkvg_save(ctx);
            vkvg_rotate(ctx, i * (PI / 6.0f));

            float b1_x = sun_radius * sin(-0.12f);
            float b1_y = -sun_radius * cos(-0.12f);
            float b2_x = sun_radius * sin(0.12f);
            float b2_y = -sun_radius * cos(0.12f);

            float tip_x = sun_radius * 1.60f * sin(0.22f);
            float tip_y = -sun_radius * 1.60f * cos(0.22f);

            vkvg_move_to(ctx, b1_x, b1_y);
            vkvg_curve_to(ctx, b1_x * 1.3f, b1_y * 1.1f, tip_x * 0.8f, tip_y * 0.9f, tip_x, tip_y);
            vkvg_curve_to(ctx, tip_x * 0.9f, tip_y * 0.95f, b2_x * 1.2f, b2_y * 1.2f, b2_x, b2_y);
            vkvg_close_path(ctx);
            vkvg_fill(ctx);

            vkvg_restore(ctx);
        }

        vkvg_arc(ctx, 0, 0, sun_radius, 0, 2.0f * PI);
        vkvg_set_source_rgba(ctx, 1.0f, 0.84f, 0.30f, 1.0f);
        vkvg_fill(ctx);
        vkvg_restore(ctx);

        // --- 🌙 THE POETIC SPHERICAL CELESTIAL MOON (True Phase Silhouette) ---
        float moon_y = -disk_r * 0.58f;
        float m_r = disk_r * 0.20f; // True radius of the lunar sphere

        vkvg_save(ctx);
        vkvg_translate(ctx, 0, moon_y);

        // 1. Draw the base illuminated golden lunar sphere
        vkvg_arc(ctx, 0, 0, m_r, 0, 2.0f * PI);
        vkvg_set_source_rgba(ctx, 0.98f, 0.94f, 0.82f, 1.0f); // Polished Cream-Gold
        vkvg_fill(ctx);

               // 2. Overlap the dark spherical shadow terminator mask using even-odd rules
               // By shifting a structural circle horizontally (e.g., m_r * 0.65f), we mimic the geometric
               // curvature of a real shadow falling across a physical round celestial body.
        vkvg_arc(ctx, m_r * 0.65f, 0, m_r * 0.98f, 0, 2.0f * PI);
        vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);

        // Use the deep midnight sky backdrop color as the shadow cast to naturally mask out the crescent
        VkvgPattern shadow_color = vkvg_pattern_create_linear(0, -m_r, 0, m_r);
        vkvg_pattern_add_color_stop(shadow_color, 0.0f, 0.005f, 0.01f, 0.05f, 1.0f);
        vkvg_pattern_add_color_stop(shadow_color, 1.0f, 0.02f, 0.04f, 0.12f, 1.0f);
        vkvg_set_source(ctx, shadow_color);
        vkvg_fill(ctx);
        vkvg_pattern_destroy(shadow_color);

        vkvg_restore(ctx);

               // --- ✨ THE SPARKLING STARFIELD ---
        for (int s = 0; s < 30; s++) {
            float seed_x = sin(s * 5721.43f) * 0.80f;
            // Kept strictly within the night hemisphere profile
            float seed_y = -0.20f - fabs(cos(s * 8219.11f)) * 0.65f;

            float sx = disk_r * seed_x;
            float sy = disk_r * seed_y;

            float star_brightness = 0.4f + fabs(sin(s * 12.5f)) * 0.6f;
            float size_variant = 0.6f + fabs(cos(s * 88.2f)) * 1.4f;

            // Draw prominent brilliant multi-pointed stars for depth
            if (s % 5 == 0) {
                vkvg_save(ctx);
                vkvg_translate(ctx, sx, sy);
                vkvg_set_source_rgba(ctx, 1.0f, 0.95f, 0.60f, star_brightness);
                float f_sz = size_variant * 2.5f;
                vkvg_move_to(ctx, 0, -f_sz);
                vkvg_line_to(ctx, f_sz * 0.2f, -f_sz * 0.2f); vkvg_line_to(ctx, f_sz, 0);
                vkvg_line_to(ctx, f_sz * 0.2f, f_sz * 0.2f);   vkvg_line_to(ctx, 0, f_sz);
                vkvg_line_to(ctx, -f_sz * 0.2f, f_sz * 0.2f);  vkvg_line_to(ctx, -f_sz, 0);
                vkvg_line_to(ctx, -f_sz * 0.2f, -f_sz * 0.2f);
                vkvg_close_path(ctx);
                vkvg_fill(ctx);
                vkvg_restore(ctx);
            } else {
                // Soft background stars
                vkvg_set_source_rgba(ctx, 0.98f, 0.88f, 0.45f, star_brightness * 0.7f);
                vkvg_arc(ctx, sx, sy, size_variant, 0, 2.0f * PI);
                vkvg_fill(ctx);
            }
        }

        vkvg_restore(ctx); // Break clear of masking clip bounds safely

               // --- 🏆 POLISHED METALLIC STRAIGHT BEZEL FRAME ---
        vkvg_set_line_width(ctx, 2.5f);
        VkvgPattern metallic_frame = vkvg_pattern_create_linear(disk_cx, comp_y, disk_cx, disk_cy);
        vkvg_pattern_add_color_stop(metallic_frame, 0.0f, 0.85f, 0.88f, 0.92f, 1.0f);
        vkvg_pattern_add_color_stop(metallic_frame, 1.0f, 0.32f, 0.35f, 0.40f, 1.0f);
        vkvg_set_source(ctx, metallic_frame);

        vkvg_arc(ctx, disk_cx, disk_cy, comp_w * 0.5f, PI, 2.0f * PI);
        vkvg_close_path(ctx);
        vkvg_stroke(ctx);
        vkvg_pattern_destroy(metallic_frame);

               // --- 🔩 DECORATIVE MECHANICAL RIVETS ---
        vkvg_set_source_rgba(ctx, 0.88f, 0.70f, 0.35f, 0.9f); // Polished Watchmaker Gold
        float rivet_y = disk_cy;
        float rivet_left_x = disk_cx - (comp_w * 0.5f) + 4.0f;
        float rivet_right_x = disk_cx + (comp_w * 0.5f) - 4.0f;
        float rivet_radius = 3.5f;

               // Left Rivet
        vkvg_arc(ctx, rivet_left_x, rivet_y, rivet_radius, 0, 2.0f * PI);
        vkvg_fill(ctx);
        vkvg_set_source_rgba(ctx, 0.1f, 0.1f, 0.1f, 0.6f);
        vkvg_set_line_width(ctx, 1.0f);
        vkvg_move_to(ctx, rivet_left_x - 2.0f, rivet_y - 1.0f);
        vkvg_line_to(ctx, rivet_left_x + 2.0f, rivet_y + 1.0f);
        vkvg_stroke(ctx);

               // Right Rivet
        vkvg_set_source_rgba(ctx, 0.88f, 0.70f, 0.35f, 0.9f);
        vkvg_arc(ctx, rivet_right_x, rivet_y, rivet_radius, 0, 2.0f * PI);
        vkvg_fill(ctx);
        vkvg_set_source_rgba(ctx, 0.1f, 0.1f, 0.1f, 0.6f);
        vkvg_move_to(ctx, rivet_right_x - 1.0f, rivet_y + 2.0f);
        vkvg_line_to(ctx, rivet_right_x + 1.0f, rivet_y - 2.0f);
        vkvg_stroke(ctx);
    }

    void draw_date_window(VkvgContext ctx, float cx, float cy, float r) {
        // Calculate proportional placement at the 3 o'clock position
        float date_box_w = r * 0.22f;
        float date_box_h = r * 0.14f;
        float date_box_x = cx + r * 0.55f - (date_box_w / 2.0f);
        float date_box_y = cy - (date_box_h / 2.0f);

               // 1. Draw the background window slot plate (Crisp White/Silver)
        vkvg_set_source_rgba(ctx, 0.95f, 0.96f, 0.98f, 1.0f);
        vkvg_rectangle(ctx, date_box_x, date_box_y, date_box_w, date_box_h);
        vkvg_fill(ctx);

               // 2. Query and bind typography
        vkvg_set_font(ctx, font);
        vkvg_set_source_rgba(ctx, 0.05f, 0.05f, 0.08f, 1.0f); // Deep ink black text

        char date_str[4];
        snprintf(date_str, sizeof(date_str), "%d", timeinfo->tm_mday);

        // Fetch text layout metrics from the font cache for precise mathematical alignment
        vkvg_text_extents_t extents;
        vkvg_text_extents(ctx, date_str, &extents);

        // Perfect centering layout rules
        float text_x = date_box_x + (date_box_w - extents.x_advance) / 2.0f;
        float text_y = date_box_y + (date_box_h + extents.height) / 2.0f - 2.0f;

        vkvg_move_to(ctx, text_x, text_y);
        vkvg_show_text(ctx, date_str);

               // 3. Draw a sharp structural inner bezel frame around the cutout
        vkvg_set_source_rgba(ctx, 0.1f, 0.12f, 0.15f, 0.8f);
        vkvg_set_line_width(ctx, 2.0f);
        vkvg_rectangle(ctx, date_box_x, date_box_y, date_box_w, date_box_h);
        vkvg_stroke(ctx);
    }
    void draw_gear(VkvgContext ctx, float x, float y, float radius, int teeth, float rotation_angle) {
        vkvg_save(ctx);
        vkvg_translate(ctx, x, y);
        vkvg_rotate(ctx, rotation_angle);

        float tooth_depth = radius * 0.12f;
        float base_radius = radius - tooth_depth / 2.0f;
        float peak_radius = radius + tooth_depth / 2.0f;
        float angle_per_tooth = (2.0f * PI) / teeth;

               // Cut out the interlocking teeth profile
        for (int i = 0; i < teeth; i++) {
            float angle = i * angle_per_tooth;

            float a1 = angle;
            float a2 = angle + angle_per_tooth * 0.35f;
            float a3 = angle + angle_per_tooth * 0.50f;
            float a4 = angle + angle_per_tooth * 0.85f;

            if (i == 0) {
                vkvg_move_to(ctx, base_radius * sin(a1), -base_radius * cos(a1));
            } else {
                vkvg_line_to(ctx, base_radius * sin(a1), -base_radius * cos(a1));
            }

            vkvg_line_to(ctx, peak_radius * sin(a2), -peak_radius * cos(a2));
            vkvg_line_to(ctx, peak_radius * sin(a3), -peak_radius * cos(a3));
            vkvg_line_to(ctx, base_radius * sin(a4), -base_radius * cos(a4));
        }
        vkvg_close_path(ctx);

               // Create mechanical holes / spokes inside the wheel framework
        vkvg_arc(ctx, 0, 0, radius * 0.75f, 0, 2 * PI);
        vkvg_arc(ctx, 0, 0, radius * 0.25f, 0, 2 * PI);

        // Use Even-Odd fill rule to cleanly hollow out the center and spaces
        vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
        vkvg_fill(ctx);

               // Add classic structural spokes
        vkvg_set_line_width(ctx, radius * 0.08f);
        for(int s = 0; s < 4; s++) {
            float spoke_angle = s * (PI / 2.0f);
            vkvg_move_to(ctx, radius * 0.24f * sin(spoke_angle), radius * 0.24f * cos(spoke_angle));
            vkvg_line_to(ctx, radius * 0.76f * sin(spoke_angle), radius * 0.76f * cos(spoke_angle));
            vkvg_stroke(ctx);
        }

               // Center jewel / brass rivet point
        vkvg_arc(ctx, 0, 0, radius * 0.12f, 0, 2 * PI);
        vkvg_set_source_rgba(ctx, 0.9f, 0.6f, 0.2f, 1.0f); // Brass/Gold
        vkvg_fill(ctx);

        vkvg_restore(ctx);
    }
    long get_smart_throttle_sleep_ms(bool sweep_enabled) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);

        long current_ms = ts.tv_nsec / 1000000;

        if (sweep_enabled) {
            long frame_target = 16;
            long sleep_time = frame_target - (current_ms % frame_target);
            return sleep_time > 0 ? sleep_time : 1;
        } else {
            long sleep_time = 1000 - current_ms;
            return sleep_time + 2;
        }
    }
};

int main(int argc, char** argv) {
    ClockApp app{};
    app.Run();
}
