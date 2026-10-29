#include "VkvgApp.hpp"
//#include "vkh_phyinfo.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdio.h>
//#include <chrono>
#include <unistd.h>
#include <fcntl.h>

bool VkvgApp::try_get_phyinfo(VkhPhyInfo* phys, uint32_t phyCount, VkPhysicalDeviceType gpuType, VkhPhyInfo* phy) {
    for (uint32_t i = 0; i < phyCount; i++) {
        if (vkh_phyinfo_get_properties(phys[i]).deviceType == gpuType) {
            *phy = phys[i];
            return true;
        }
    }
    return false;
}

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "vkengine: GLFW error %d: %s\n", error, description);
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

void VkvgApp::Init() {
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
            std::cout << "\t" << i << ": " << vkh_phyinfo_get_properties(phys[i]).deviceName << std::endl;
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

    memory_properties = vkh_phyinfo_get_memory_properties(pi);
    gpu_props         = vkh_phyinfo_get_properties(pi);

    uint32_t qCount        = 0;
    float    qPriorities[] = {0.0};

    VkDeviceQueueCreateInfo pQueueInfos[] = {{}, {}, {}};
    if (vkh_phyinfo_create_presentable_queues(pi, 1, qPriorities, &pQueueInfos[qCount]))
        qCount++;

    enabledExtsCount = 0;

    if (vkvg_get_required_device_extensions(phy, enabledExts, &enabledExtsCount) != VKVG_STATUS_SUCCESS) {
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
}
void VkvgApp::Run() {

    Init();

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
void VkvgApp::CleanUp() {
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

void VkvgApp::OnKey(int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS)
        return;
    switch (key) {
    case GLFW_KEY_SPACE:
        // paused = !paused;
        break;
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(win, GLFW_TRUE);
        break;
    }
}
void VkvgApp::OnScroll(double xoffset, double yoffset) {
    double zoomFactor = (yoffset > 0) ? 1.15 : (1.0 / 1.15);
    double mouseX, mouseY;
    glfwGetCursorPos(win, &mouseX, &mouseY);

    offsetX = mouseX - (mouseX - offsetX) * zoomFactor;
    offsetY = mouseY - (mouseY - offsetY) * zoomFactor;
    zoom *= zoomFactor;
    refresh = true;
}
void VkvgApp::OnMouseButton (int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            isDragging = true;
            glfwGetCursorPos(win, &lastMouseX, &lastMouseY);
        } else if (action == GLFW_RELEASE) {
            isDragging = false;
        }
    }
}
void VkvgApp::OnMouseMove (double xpos, double ypos) {
    if (isDragging) {
        double deltaX = xpos - lastMouseX;
        double deltaY = ypos - lastMouseY;
        offsetX += deltaX;
        offsetY += deltaY;
        lastMouseX = xpos;
        lastMouseY = ypos;
        refresh = true;
    }
}

void VkvgApp::Render() {
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_source_rgba(ctx, 0.96f, 0.95f, 0.94f, 1.0f);
    vkvg_paint(ctx);

    vkvg_translate(ctx, offsetX, offsetY);
    vkvg_scale(ctx, zoom, zoom);

    vkvg_rectangle(ctx, 50,50,50,50);
    vkvg_set_source_rgb(ctx, 0, 1, 0);
    vkvg_fill(ctx);

    vkvg_destroy(ctx);

    refresh = false;
}

