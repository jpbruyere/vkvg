
#include "OsmApp.hpp"
#include "vkh_phyinfo.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdio.h>
//#include <chrono>
#include <unistd.h>
#include <fcntl.h>
#include <osmium/io/reader.hpp>
#include <osmium/io/pbf_input.hpp>
#include <osmium/visitor.hpp>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/geom/mercator_projection.hpp>
#include <osmium/io/xml_input.hpp>  // <-- OPTIONAL: Adds XML parsing support (.osm)

using index_type = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

class CacheHandler : public osmium::handler::Handler {
    std::vector<MapWay>& target_ways;
    std::vector<MapNode>& target_nodes;

    RoadType parse_road_type(const char* highway) {
        if (!highway) return RoadType::Unknown;
        if (strcmp(highway, "motorway") == 0 || strcmp(highway, "primary") == 0) return RoadType::Primary;
        if (strcmp(highway, "secondary") == 0) return RoadType::Secondary;
        if (strcmp(highway, "tertiary") == 0) return RoadType::Tertiary;
        if (strcmp(highway, "residential") == 0) return RoadType::Residential;
        return RoadType::Service;
    }

  public:
    CacheHandler(std::vector<MapWay>& w, std::vector<MapNode>& n) : target_ways(w), target_nodes(n) {}

           // 1. COLLECT HIGH-VALUE POIs
    void node(const osmium::Node& node) {
        const char* amenity = node.tags().get_value_by_key("amenity");
        const char* shop = node.tags().get_value_by_key("shop");
        const char* tourism = node.tags().get_value_by_key("tourism");

        if (amenity || shop || tourism) {
            MapNode mn;
            mn.coord = osmium::geom::lonlat_to_mercator(node.location());
            mn.name = node.tags().get_value_by_key("name") ? node.tags().get_value_by_key("name") : "";
            mn.type = amenity ? amenity : (shop ? shop : tourism);
            target_nodes.push_back(mn);
        }
    }

           // 2. COLLECT ROADS AND LANDUSE POLYGONS
    void way(const osmium::Way& way) {
        if (way.nodes().size() < 2) return;

        MapWay w;
        const char* highway = way.tags().get_value_by_key("highway");
        const char* building = way.tags().get_value_by_key("building");
        const char* natural = way.tags().get_value_by_key("natural");
        const char* water = way.tags().get_value_by_key("water");
        const char* name = way.tags().get_value_by_key("name");

        w.is_building = (building != nullptr);
        w.is_water = (water || (natural && strcmp(natural, "water") == 0));
        w.road_type = parse_road_type(highway);
        w.name = name ? name : "";

        for (const auto& node : way.nodes()) {
            if (node.location().valid()) {
                w.points.push_back(osmium::geom::lonlat_to_mercator(node.location()));
            }
        }
        target_ways.push_back(w);
    }
};

bool OsmApp::try_get_phyinfo(VkhPhyInfo* phys, uint32_t phyCount, VkPhysicalDeviceType gpuType, VkhPhyInfo* phy) {
    for (uint32_t i = 0; i < phyCount; i++) {
        if (phys[i]->properties.deviceType == gpuType) {
            *phy = phys[i];
            return true;
        }
    }
    return false;
}

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "vkengine: GLFW error %d: %s\n", error, description);
}

void OsmApp::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS)
        return;
    switch (key) {
    case GLFW_KEY_SPACE:
        // paused = !paused;
        break;
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    }
}
void OsmApp::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    OsmApp* app = reinterpret_cast<OsmApp*>(glfwGetWindowUserPointer(window));
    double zoomFactor = (yoffset > 0) ? 1.15 : (1.0 / 1.15);

    // Zoom centered roughly around current cursor position
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    app->offsetX = mouseX - (mouseX - app->offsetX) * zoomFactor;
    app->offsetY = mouseY - (mouseY - app->offsetY) * zoomFactor;
    app->zoom *= zoomFactor;
    app->refresh = true;
}
void OsmApp::mouse_move_callback(GLFWwindow* window, double xpos, double ypos) {
    OsmApp* app = reinterpret_cast<OsmApp*>(glfwGetWindowUserPointer(window));
    if (app->isDragging) {
        double deltaX = xpos - app->lastMouseX;
        double deltaY = ypos - app->lastMouseY;
        app->offsetX += deltaX;
        app->offsetY += deltaY;
        app->lastMouseX = xpos;
        app->lastMouseY = ypos;
        app->refresh = true;
    }
}
void OsmApp::mouse_button_callback(GLFWwindow* window, int button, int action, int modif) {
    OsmApp* app = reinterpret_cast<OsmApp*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            app->isDragging = true;
            glfwGetCursorPos(window, &app->lastMouseX, &app->lastMouseY);
        } else if (action == GLFW_RELEASE) {
            app->isDragging = false;
        }
    }
}

static bool is_desktop_environment_available(void) {
#if defined(__ANDROID__)
        // Android always runs inside an active graphical framework handled by SurfaceFlinger.
        // If the binary compiles for Android, a display context is inherently present.
        return true;
#elif defined(_WIN32) || defined(_WIN64)
        // Windows always has a desktop environment unless running as a headless NT Service.
        return getenv("SystemRoot") != NULL;

#elif defined(__APPLE__) && defined(__MACH__)
        // macOS always runs within a Quartz GUI session.
        return getenv("HOME") != NULL;

#else
        // Linux/Unix (Debian, etc.)
        // Check for active X11 display handles or Wayland compositor paths
        bool has_x11     = getenv("DISPLAY") != NULL;
        bool has_wayland = getenv("WAYLAND_DISPLAY") != NULL;
        return (has_x11 || has_wayland);
#endif
    }

void OsmApp::Init() {
    //vkh_log_level = VKH_LOG_FULL;

    displayMode = !is_desktop_environment_available();

    if (!displayMode) {
        glfwSetErrorCallback(glfw_error_callback);

        if (!glfwInit()) {
            perror("glfwInit failed");
            exit(-1);
        }

        if (!glfwVulkanSupported()) {
            perror("glfwVulkanSupported return false.");
            exit(-1);
        }
    }

    const char* enabledLayers[10];
    const char* enabledExts[10];
    uint32_t    enabledExtsCount = 0, enabledLayersCount = 0, phyCount = 0;

    vkh_layers_check_init();
#ifdef VKVG_USE_VALIDATION
    if (vkh_layer_is_present("VK_LAYER_KHRONOS_validation"))
        enabledLayers[enabledLayersCount++] = "VK_LAYER_KHRONOS_validation";
#endif
#ifdef VKVG_USE_MESA_OVERLAY
    if (vkh_layer_is_present("VK_LAYER_MESA_overlay"))
        enabledLayers[enabledLayersCount++] = "VK_LAYER_MESA_overlay";
#endif

#ifdef VKVG_USE_RENDERDOC
    if (vkh_layer_is_present("VK_LAYER_RENDERDOC_Capture"))
        enabledLayers[enabledLayersCount++] = "VK_LAYER_RENDERDOC_Capture";
#endif
    vkh_layers_check_release();

    vkvg_get_required_instance_extensions(enabledExts, &enabledExtsCount);

    if (displayMode) {
        enabledExts[enabledExtsCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        enabledExts[enabledExtsCount++] = VK_KHR_DISPLAY_EXTENSION_NAME;
    } else {
        uint32_t     glfwReqExtsCount = 0;
        const char** gflwExts         = glfwGetRequiredInstanceExtensions(&glfwReqExtsCount);
        for (uint32_t i = 0; i < glfwReqExtsCount; i++)
            enabledExts[i + enabledExtsCount] = gflwExts[i];
        enabledExtsCount += glfwReqExtsCount;
    }

    app = vkh_app_create(1, 2, "vkvg", enabledLayersCount, enabledLayers, enabledExtsCount, enabledExts);

#if defined(DEBUG) && defined(VKVG_DBG_UTILS)
    uint32_t severity = 0;
    for (const uint32_t& s : logSeverity)
        severity += pow(16, s);

    vkh_app_enable_debug_messenger(app, (VkDebugUtilsMessageTypeFlagBitsEXT)logType,
                                   (VkDebugUtilsMessageSeverityFlagBitsEXT)severity, NULL);
#endif

    VkhPhyInfo* phys = vkh_app_get_phyinfos(app, &phyCount, NULL);

    if (listGpus) {
        std::cout << "Available GPU's:" << std::endl;
        std::cout << "================" << std::endl;
        for (uint32_t i = 0; i < phyCount; i++) {
            std::cout << "\t" << i << ": " << phys[i]->properties.deviceName << std::endl;
        }
        vkh_app_free_phyinfos(phyCount, phys);
        vkh_app_destroy(app);
        glfwTerminate();
        exit(0);
    }

    if (!displayMode) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

        win = glfwCreateWindow((int)width, (int)height, "Window Title", NULL, NULL);

        glfwCreateWindowSurface(vkh_app_get_inst(app), win, NULL, &vkSurf);
        glfwSetKeyCallback(win, key_callback);
        glfwSetWindowUserPointer(win, this);
        glfwSetMouseButtonCallback(win, mouse_button_callback);
        glfwSetCursorPosCallback(win, mouse_move_callback);
        glfwSetScrollCallback(win, scroll_callback);
    }

    VkhPhyInfo pi = 0;
    if (gpuIndex >= 0 && gpuIndex < phyCount) {
        pi = phys[gpuIndex];
    } else {
        if (!try_get_phyinfo(phys, phyCount, preferedGPU, &pi) &&
            !try_get_phyinfo(phys, phyCount, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, &pi) &&
            !try_get_phyinfo(phys, phyCount, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, &pi))
            pi = phys[0];
    }
    assert(pi && "No vulkan physical device found.");

    VkPhysicalDevice phy = vkh_phyinfo_get_phy(pi);
    vkh_app_free_phyinfos (phyCount, phys);

    if (displayMode) {
        uint32_t display_count = 0;
        vkGetPhysicalDeviceDisplayPropertiesKHR(phy, &display_count, NULL);

        if (display_count == 0) {
            perror("No dispaly found.\n");
            return;
        }

        VkDisplayPropertiesKHR* display_props =
            (VkDisplayPropertiesKHR*)malloc(display_count * sizeof(VkDisplayPropertiesKHR));
        vkGetPhysicalDeviceDisplayPropertiesKHR(phy, &display_count, display_props);
        std::cout << "Available Vulkan Displays:" << std::endl;
        std::cout << "=========================" << std::endl;
        for (uint32_t i = 0; i < display_count; i++) {
            std::cout << i << ": " << display_props[i].displayName << std::endl;
            std::cout << "\tResolution:" << display_props[i].physicalDimensions.width <<
                " X " << display_props[i].physicalDimensions.height << std::endl;
        }
        int dispIdx;
        while (std::cout << "Enter (0-" << display_count - 1 << "): " &&
               (!(std::cin >> dispIdx) || dispIdx < 0 || dispIdx >= display_count)) {
            std::cin.clear(); std::cin.ignore(256, '\n');
        }
        VkDisplayKHR native_display = display_props[dispIdx].display;
        uint32_t mode_count = 0;
        vkGetDisplayModePropertiesKHR(phy, native_display, &mode_count, NULL);

        VkDisplayModePropertiesKHR* mode_props = (VkDisplayModePropertiesKHR*)
            malloc(mode_count * sizeof(VkDisplayModePropertiesKHR));
        vkGetDisplayModePropertiesKHR(phy, native_display, &mode_count, mode_props);

        std::cout << "Available Modes:" << std::endl;
        std::cout << "================" << std::endl;
        for (uint32_t i = 0; i < mode_count; i++) {
            VkDisplayModeParametersKHR params = mode_props[i].parameters;
            // Refresh rate is stored as millihertz (mHz), divide by 1000 for standard Hz
            float refresh_rate = (float)params.refreshRate / 1000.0f;
            std::cout << i << ": " << params.visibleRegion.width << " X " <<
                   params.visibleRegion.height << " @ " << refresh_rate << std::endl;
        }
        int modeIdx;
        while (std::cout << "Enter (0-" << mode_count - 1 << "): " &&
               (!(std::cin >> modeIdx) || modeIdx < 0 || modeIdx >= mode_count)) {
            std::cin.clear(); std::cin.ignore(256, '\n');
        }
        VkDisplayModeKHR display_mode = mode_props[modeIdx].displayMode;
        uint32_t plane_count = 0;
        vkGetPhysicalDeviceDisplayPlanePropertiesKHR(phy, &plane_count, NULL);
        VkDisplayPlanePropertiesKHR* plane_props = (VkDisplayPlanePropertiesKHR*)
            malloc(plane_count * sizeof(VkDisplayPlanePropertiesKHR));
        vkGetPhysicalDeviceDisplayPlanePropertiesKHR(phy, &plane_count, plane_props);
        uint32_t active_plane_index = 0;
        VkBool32 plane_supported = VK_FALSE;
        for (uint32_t p = 0; p < plane_count; p++) {
            vkGetDisplayPlaneSupportedDisplaysKHR(phy, p, &display_count, NULL);
            if (display_count > 0) {
                VkDisplayKHR* supported_displays = (VkDisplayKHR*)
                    malloc(display_count * sizeof(VkDisplayKHR));
                vkGetDisplayPlaneSupportedDisplaysKHR(phy, p, &display_count, supported_displays);
                for (uint32_t d = 0; d < display_count; d++) {
                    if (supported_displays[d] == native_display) {
                        active_plane_index = p;
                        plane_supported = VK_TRUE;
                        break;
                    }
                }
                free(supported_displays);
            }
            if (plane_supported) break;
        }
        if (!plane_supported) {
            fprintf(stderr, "Critical Error: Could not resolve a hardware plane that supports the selected display!\n");
            return;
        }
        VkExtent2D extent = mode_props[modeIdx].parameters.visibleRegion;
        VkDisplaySurfaceCreateInfoKHR surface_info = {
            .sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR,
            .displayMode = display_mode,
            .planeIndex = active_plane_index,
            .planeStackIndex = 0,
            .transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .globalAlpha = 1.0f,
            .alphaMode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR,
            .imageExtent = extent
        };

        vkCreateDisplayPlaneSurfaceKHR(vkh_app_get_inst(app), &surface_info, NULL, &vkSurf);
        width = extent.width;
        height = extent.height;

        free(plane_props);
        free(mode_props);
        free(display_props);
    }

    pi = vkh_phyinfo_create(phy, vkSurf);

    memory_properties = pi->memProps;
    gpu_props         = pi->properties;

    uint32_t qCount        = 0;
    float    qPriorities[] = {0.0};

    VkDeviceQueueCreateInfo pQueueInfos[] = {{}, {}, {}};
    if (vkh_phyinfo_create_presentable_queues(pi, 1, qPriorities, &pQueueInfos[qCount]))
        qCount++;

    enabledExtsCount = 0;

    if (vkvg_get_required_device_extensions(pi->phy, enabledExts, &enabledExtsCount) != VKVG_STATUS_SUCCESS) {
        perror("vkvg_get_required_device_extensions failed, enable log for details.\n");
        exit(-1);
    }
    TRY_LOAD_DEVICE_EXT(VK_KHR_swapchain)

    VkPhysicalDeviceFeatures enabledFeatures{};
    const void*              pNext = vkvg_get_device_requirements(&enabledFeatures);

    VkDeviceCreateInfo device_info = {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = pNext,
        .queueCreateInfoCount    = qCount,
        .pQueueCreateInfos       = (VkDeviceQueueCreateInfo*)&pQueueInfos,
        .enabledExtensionCount   = enabledExtsCount,
        .ppEnabledExtensionNames = enabledExts,
        .pEnabledFeatures        = &enabledFeatures,
    };

    vkhDev = vkh_device_create(app, pi, &device_info);
    int32_t gQFamIdx;
    vkh_phyinfo_get_queue_fam_indices(pi, &gQFamIdx, NULL, NULL, NULL);
    presentQIndex = gQFamIdx;
    vkh_phyinfo_destroy (pi);

    VkFormat targetFormat = VK_FORMAT_B8G8R8A8_SRGB;
    if (displayMode) {
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(phy, vkSurf, &formatCount, NULL);
        VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*
                                       )malloc(formatCount * sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(phy, vkSurf, &formatCount, formats);
        targetFormat = formats[0].format;
        free(formats);
    }

    renderer = vkh_presenter_create(vkhDev, presentQIndex, vkSurf, width, height, targetFormat, presentMode);

    if (!displayMode) {
        glfwSetKeyCallback(win, key_callback);
        glfwSetMouseButtonCallback(win, mouse_button_callback);
        glfwSetCursorPosCallback(win, mouse_move_callback);
    }
    vkvg_device_create_info_t info = {
        vkh_app_get_inst(app),
        vkh_device_get_phy(vkhDev),
        vkh_device_get_vkdev(vkhDev),
        presentQIndex, 0, false
    };

    device = vkvg_device_create(&info);
    surf   = vkvg_surface_create(device, width, height);
    vkvg_surface_set_dpy(surf, 96, 96);

    vkh_presenter_build_blit_cmd(renderer, vkvg_surface_get_vk_image(surf), width, height);

    vkDeviceWaitIdle(vkh_device_get_vkdev(vkhDev));

    // Append this PBF data-caching step to the bottom of Init():
    osmium::io::Reader reader{"/mnt/data/osm/ciney.pbf"};
    osmium::io::Header header = reader.header();
    osmium::Box box = header.joined_boxes();
    if (box.valid()) {
        osmium::geom::Coordinates min_corner = osmium::geom::lonlat_to_mercator(box.bottom_left());
        osmium::geom::Coordinates max_corner = osmium::geom::lonlat_to_mercator(box.top_right());
        min_merc_x = min_corner.x;
        max_merc_y = max_corner.y;

        double map_width_meters = max_corner.x - min_corner.x;
        double map_height_meters = max_corner.y - min_corner.y;
        double scale_x = width / map_width_meters;
        double scale_y = height / map_height_meters;
        base_scale = (scale_x < scale_y) ? scale_x : scale_y;
        base_scale *= 0.95;
    }

    index_type index;
    location_handler_type location_handler{index};
    CacheHandler cache_handler{parsed_ways, parsed_nodes};
    osmium::apply(reader, location_handler, cache_handler);
    reader.close();
}

/*void OsmApp::Render () {
    VkvgContext ctx = vkvg_create(surf);

    // Clear backbuffer surface with a solid clean baseline canvas
    vkvg_set_source_rgba(ctx, 0.95f, 0.95f, 0.95f, 1.0f);
    vkvg_paint(ctx);

           // Apply viewport transformation operations dynamically
    vkvg_translate(ctx, offsetX, offsetY);
    vkvg_scale(ctx, zoom, zoom);

           // Render pass 1: Buildings
    for (const auto& way : parsed_ways) {
        if (!way.is_building || way.points.empty()) continue;

        double start_x = (way.points[0].x - min_merc_x) * base_scale;
        double start_y = (max_merc_y - way.points[0].y) * base_scale;
        vkvg_move_to(ctx, start_x, start_y);

        for (size_t i = 1; i < way.points.size(); ++i) {
            double x = (way.points[i].x - min_merc_x) * base_scale;
            double y = (max_merc_y - way.points[i].y) * base_scale;
            vkvg_line_to(ctx, x, y);
        }
        vkvg_close_path(ctx);
        vkvg_set_source_rgba(ctx, 0.8f, 0.75f, 0.75f, 1.0f); // Sleek building grey
        vkvg_fill(ctx);
    }

           // Render pass 2: Roads (Drawn on top to respect visual hierarchy layers)
    for (const auto& way : parsed_ways) {
        if (!way.is_highway || way.points.empty()) continue;

        double start_x = (way.points[0].x - min_merc_x) * base_scale;
        double start_y = (max_merc_y - way.points[0].y) * base_scale;
        vkvg_move_to(ctx, start_x, start_y);

        for (size_t i = 1; i < way.points.size(); ++i) {
            double x = (way.points[i].x - min_merc_x) * base_scale;
            double y = (max_merc_y - way.points[i].y) * base_scale;
            vkvg_line_to(ctx, x, y);
        }
        vkvg_set_source_rgba(ctx, 0.25f, 0.25f, 0.25f, 1.0f); // Crisp dark streets
        vkvg_set_line_width(ctx, 2.0 / zoom); // Maintain consistent stroke weight relative to zoom
        vkvg_stroke(ctx);
    }

    vkvg_destroy(ctx);
    refresh = false;
}*/
void OsmApp::Render() {
    VkvgContext ctx = vkvg_create(surf);

    // Clear Background: Soft Landscape Beige
    vkvg_set_source_rgba(ctx, 0.96f, 0.95f, 0.94f, 1.0f);
    vkvg_paint(ctx);

    vkvg_translate(ctx, offsetX, offsetY);
    vkvg_scale(ctx, zoom, zoom);

           // --- PASS 1: WATER FEATURES ---
    for (const auto& way : parsed_ways) {
        if (!way.is_water || way.points.empty()) continue;
        vkvg_move_to(ctx, (way.points[0].x - min_merc_x) * base_scale, (max_merc_y - way.points[0].y) * base_scale);
        for (size_t i = 1; i < way.points.size(); ++i) {
            vkvg_line_to(ctx, (way.points[i].x - min_merc_x) * base_scale, (max_merc_y - way.points[i].y) * base_scale);
        }
        vkvg_close_path(ctx);
        vkvg_set_source_rgba(ctx, 0.67f, 0.81f, 0.93f, 1.0f); // Soft Hydro Blue
        vkvg_fill(ctx);
    }

           // --- PASS 3: MULTI-TIER HIGHWAY INFRASTRUCTURE ---
    for (const auto& way : parsed_ways) {
        if (way.road_type == RoadType::Unknown || way.points.empty()) continue;

        vkvg_move_to(ctx, (way.points[0].x - min_merc_x) * base_scale, (max_merc_y - way.points[0].y) * base_scale);
        for (size_t i = 1; i < way.points.size(); ++i) {
            vkvg_line_to(ctx, (way.points[i].x - min_merc_x) * base_scale, (max_merc_y - way.points[i].y) * base_scale);
        }

               // Apply dynamic sizing & color templates based on road priority tier
        switch (way.road_type) {
        case RoadType::Primary:
            vkvg_set_source_rgba(ctx, 0.98f, 0.69f, 0.61f, 1.0f); // Primary Orange/Red
            vkvg_set_line_width(ctx, 5.0);
            break;
        case RoadType::Secondary:
            vkvg_set_source_rgba(ctx, 0.99f, 0.85f, 0.59f, 1.0f); // Secondary Gold Yellow
            vkvg_set_line_width(ctx, 3.5);
            break;
        case RoadType::Tertiary:
            vkvg_set_source_rgba(ctx, 1.0f, 1.0f, 1.0f, 1.0f);    // White trunk lines
            vkvg_set_line_width(ctx, 3.0);
            break;
        case RoadType::Residential:
            vkvg_set_source_rgba(ctx, 1.0f, 1.0f, 1.0f, 1.0f);    // Thin White streets
            vkvg_set_line_width(ctx, 2.0);
            break;
        default:
            vkvg_set_source_rgba(ctx, 0.9f, 0.9f, 0.9f, 1.0f);     // Service lanes (Dull grey)
            vkvg_set_line_width(ctx, 1.0);
            break;
        }
        vkvg_stroke(ctx);
    }
    // BUILDINGS ---
    for (const auto& way : parsed_ways) {
        if (!way.is_building || way.points.empty()) continue;
        vkvg_move_to(ctx, (way.points[0].x - min_merc_x) * base_scale, (max_merc_y - way.points[0].y) * base_scale);
        for (size_t i = 1; i < way.points.size(); ++i) {
            vkvg_line_to(ctx, (way.points[i].x - min_merc_x) * base_scale, (max_merc_y - way.points[i].y) * base_scale);
        }
        vkvg_close_path(ctx);
        vkvg_set_source_rgba(ctx, 0.85f, 0.82f, 0.80f, 1.0f); // Slate Grey-Orange fill
        vkvg_fill_preserve(ctx);
        vkvg_set_source_rgba(ctx, 0.75f, 0.70f, 0.68f, 1.0f); // Dark boundary outline
        vkvg_set_line_width(ctx, 0.1);
        vkvg_stroke(ctx);
    }

    // --- PASS 4: LANDMARK TEXT LABELS & ICON OVERLAYS ---
    VkvgFont font = vkvg_font_create(surf, "sans-serif:bold", 12);
    vkvg_set_font(ctx,font);

    // Only display smaller nodes and labels once the view is sufficiently zoomed in
    if (zoom > 2.5) {
        for (const auto& node : parsed_nodes) {
            double scr_x = (node.coord.x - min_merc_x) * base_scale;
            double scr_y = (max_merc_y - node.coord.y) * base_scale;

            render_svg_icon(ctx, node.type, scr_x, scr_y);

            /*if (!node.name.empty()) {
                vkvg_set_font_size(ctx, 11.0 / zoom);
                vkvg_set_source_rgba(ctx, 0.2f, 0.2f, 0.2f, 1.0f); // Charcoal clean font text

                vkvg_move_to(ctx, scr_x + (5.0 / zoom), scr_y + (12.0 / zoom));
                vkvg_show_text(ctx, node.name.c_str());
            }*/
        }
    }

    vkvg_destroy(ctx);
    vkvg_font_destroy(font);

    refresh = false;
}

void OsmApp::render_svg_icon(VkvgContext ctx, const std::string& type, double x, double y) {
    printf("%s\n",type.c_str());
}

void OsmApp::Run() {
    if (displayMode)
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    else
        glfwSetWindowTitle(win, "Open Street Map");

    while (true) {
        if (displayMode) {
            char ch;
            if (read(STDIN_FILENO, &ch, 1) > 0) {
                if (ch == 'q') break;
                printf("Key pressed: %c\r\n", ch);
            }
        } else {
            glfwPollEvents();
            if (glfwWindowShouldClose(win))
                break;
        }

        if (refresh)
            Render();

        if (!vkh_presenter_draw(renderer)) {
            vkh_presenter_get_size(renderer, &width, &height);
            vkvg_surface_destroy(surf);
            surf = vkvg_surface_create(device, width, height);
            vkvg_surface_set_dpy(surf, 96, 96);
            vkh_presenter_build_blit_cmd(renderer, vkvg_surface_get_vk_image(surf), width, height);
            vkDeviceWaitIdle(vkh_device_get_vkdev(vkhDev));
            refresh = true;
            continue;
        }
    }
}

void OsmApp::CleanUp() {
    if (save_img) {
        std::string path = "output.png";
        vkvg_surface_write_to_png(surf, path.c_str());
    }
    vkvg_surface_destroy(surf);
    vkvg_device_destroy(device);

    vkDeviceWaitIdle(vkh_device_get_vkdev(vkhDev));
    vkh_presenter_destroy(renderer);
    vkDestroySurfaceKHR(vkh_app_get_inst(app), vkSurf, NULL);

    vkh_device_destroy(vkhDev);
    if (displayMode) {
        vkh_app_destroy(app);
    } else {
        glfwDestroyWindow(win);
        vkh_app_destroy(app);
        glfwTerminate();
    }
}
