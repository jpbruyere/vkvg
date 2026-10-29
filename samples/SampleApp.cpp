
#include "SampleApp.hpp"
#include "vkh_phyinfo.h"

#include <GLFW/glfw3.h>

#include "VkvgTest.hpp"

#include <iostream>
#include <stdio.h>
#include <chrono>
#include <unistd.h>
#include <fcntl.h>

bool SampleApp::try_get_phyinfo(VkhPhyInfo* phys, uint32_t phyCount, VkPhysicalDeviceType gpuType, VkhPhyInfo* phy) {
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
void SampleApp::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
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
static void char_callback(GLFWwindow* window, uint32_t c) {}
static void mouse_move_callback(GLFWwindow* window, double x, double y) {}
static void scroll_callback(GLFWwindow* window, double x, double y) {}
static void mouse_button_callback(GLFWwindow* window, int but, int state, int modif) {}

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

void SampleApp::Init() {
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

    vkhDev        = vkh_device_create(app, pi, &device_info);
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
}

void SampleApp::Run() {
    uint32_t curIter = 0;

    if (VkvgTest::tests.empty())
        return;

    int       testIdx = 0;
    auto      it      = VkvgTest::tests.begin();
    VkvgTest* curTest = NULL;
    std::vector<double> run_time_values;
    run_time_values.reserve(iterations);
    double totTime = 0;

    if (testsToRun.empty()) {
        curTest = (*it);
    } else {
        curTest = VkvgTest::tests[testsToRun[testIdx]];
    }

    if (displayMode)
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    else
        glfwSetWindowTitle(win, curTest->name.c_str());

    curTest->initTest(this);
    curTest->performTest(testIdx);//warm-up

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

        int idx = testIdx;
        auto start = std::chrono::steady_clock::now();
        DoNotOptimize(idx);
        int res = curTest->performTest(idx);
        DoNotOptimize(res);
        auto finish = std::chrono::steady_clock::now();

        auto elapsed = finish - start;
        /*double elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                     elapsed).count();*/
        double elapsed_ms = std::chrono::duration<double, std::milli>(elapsed).count();


        if (!vkh_presenter_draw(renderer)) {
            vkh_presenter_get_size(renderer, &width, &height);
            curTest->cleanTest(0, nullptr);
            curTest->initTest(this);
            curTest->performTest(testIdx);//warm-up
            continue;
        }

        if (iterations > 0) {
            totTime += elapsed_ms;
            run_time_values.push_back(elapsed_ms);

            if (++curIter == iterations) {
                curTest->cleanTest(totTime, run_time_values.data());
                testIdx++;
                if (testsToRun.empty()) {
                    if (++it == VkvgTest::tests.end()) {
                        curTest = NULL;
                        break;
                    }
                    curTest = (*it);
                } else {
                    if (testIdx == testsToRun.size()) {
                        curTest = NULL;
                        break;
                    }
                    curTest = VkvgTest::tests[testsToRun[testIdx]];
                }

                curIter = 0;
                totTime = 0;
                run_time_values.clear();
                run_time_values.reserve(iterations);

                if (!displayMode)
                    glfwSetWindowTitle(win, curTest->name.c_str());

                curTest->initTest(this);
                curTest->performTest(testIdx);//warm-up
            }
        }
    }
    if (curTest != NULL)
        curTest->cleanTest(totTime, run_time_values.data());
}

void SampleApp::CleanUp() {
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
