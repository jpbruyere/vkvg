#include <iostream>
#include <stdio.h>

#include "vkvg.h"
#include "vkh.h"
#include "vkh_phyinfo.h"

#include <GLFW/glfw3.h>

#include "SampleApp.hpp"
#include "VkvgTest.hpp"



bool SampleApp::try_get_phyinfo(VkhPhyInfo *phys, uint32_t phyCount, VkPhysicalDeviceType gpuType, VkhPhyInfo *phy) {
    for (uint32_t i = 0; i < phyCount; i++) {
        if (phys[i]->properties.deviceType == gpuType) {
            *phy = phys[i];
            return true;
        }
    }
    return false;
}

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "vkengine: GLFW error %d: %s\n", error, description);
}
void SampleApp::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS)
        return;
    switch (key) {
    case GLFW_KEY_SPACE:
        //paused = !paused;
        break;
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    }
}
static void char_callback(GLFWwindow *window, uint32_t c) {}
static void mouse_move_callback(GLFWwindow *window, double x, double y) {}
static void scroll_callback(GLFWwindow *window, double x, double y) {}
static void mouse_button_callback(GLFWwindow *window, int but, int state, int modif) {}


void SampleApp::Init() {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        perror("glfwInit failed");
        exit(-1);
    }

    if (!glfwVulkanSupported()) {
        perror("glfwVulkanSupported return false.");
        exit(-1);
    }
    const char *enabledLayers[10];
    const char *enabledExts[10];
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

    uint32_t     glfwReqExtsCount = 0;
    const char **gflwExts         = glfwGetRequiredInstanceExtensions(&glfwReqExtsCount);

    vkvg_get_required_instance_extensions(enabledExts, &enabledExtsCount);

    for (uint32_t i = 0; i < glfwReqExtsCount; i++)
        enabledExts[i + enabledExtsCount] = gflwExts[i];

    enabledExtsCount += glfwReqExtsCount;

    app = vkh_app_create(1, 2, "vkvg", enabledLayersCount, enabledLayers, enabledExtsCount, enabledExts);

#if defined(DEBUG) && defined(VKVG_DBG_UTILS)
    uint32_t severity = 0;
    for(const uint32_t& s : logSeverity)
        severity += pow(16, s);

    vkh_app_enable_debug_messenger(app,
                                   (VkDebugUtilsMessageTypeFlagBitsEXT)logType,
                                   (VkDebugUtilsMessageSeverityFlagBitsEXT)severity,
                                   NULL);
#endif
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    win = glfwCreateWindow((int)width, (int)height, "Window Title", NULL, NULL);

    glfwCreateWindowSurface(vkh_app_get_inst(app), win, NULL, &vkSurf);

    VkhPhyInfo *phys = vkh_app_get_phyinfos(app, &phyCount, vkSurf);

    if (listGpus) {
        std::cout << "Available GPU's:" << std::endl;
        std::cout << "================" << std::endl;
        for (uint32_t i = 0; i < phyCount; i++) {
            std::cout << "\t" << i << ": " << phys[i]->properties.deviceName << std::endl;
        }
        vkh_app_free_phyinfos(phyCount, phys);
        vkDestroySurfaceKHR(vkh_app_get_inst(app), vkSurf, NULL);
        glfwDestroyWindow(win);
        vkh_app_destroy(app);
        glfwTerminate();
        exit(0);
    }

    VkhPhyInfo pi = 0;
    if (gpuIndex > 0 && gpuIndex < phyCount) {
        pi = phys[gpuIndex];
    } else {
        if (!try_get_phyinfo(phys, phyCount, preferedGPU, &pi) &&
            !try_get_phyinfo(phys, phyCount, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, &pi) &&
            !try_get_phyinfo(phys, phyCount, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, &pi))
            pi = phys[0];
    }
    assert(pi && "No vulkan physical device found.");

    memory_properties = pi->memProps;
    gpu_props         = pi->properties;

    uint32_t qCount        = 0;
    float    qPriorities[] = {0.0};

    VkDeviceQueueCreateInfo pQueueInfos[] = {{}, {}, {}};
    if (vkh_phyinfo_create_presentable_queues(pi, 1, qPriorities, &pQueueInfos[qCount]))
        qCount++;
    /*if (vkh_phyinfo_create_compute_queues		(pi, 1, qPriorities, &pQueueInfos[qCount]))
        qCount++;
    if (vkh_phyinfo_create_transfer_queues		(pi, 1, qPriorities, &pQueueInfos[qCount]))
        qCount++;*/

    enabledExtsCount = 0;

    if (vkvg_get_required_device_extensions(pi->phy, enabledExts, &enabledExtsCount) != VKVG_STATUS_SUCCESS) {
        perror("vkvg_get_required_device_extensions failed, enable log for details.\n");
        exit(-1);
    }
    TRY_LOAD_DEVICE_EXT(VK_KHR_swapchain)

    VkPhysicalDeviceFeatures enabledFeatures{};
    const void *pNext = vkvg_get_device_requirements(&enabledFeatures);

    VkDeviceCreateInfo device_info = {.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                      .pNext                   = pNext,
                                      .queueCreateInfoCount    = qCount,
                                      .pQueueCreateInfos       = (VkDeviceQueueCreateInfo *)&pQueueInfos,
                                      .enabledExtensionCount   = enabledExtsCount,
                                      .ppEnabledExtensionNames = enabledExts,
                                      .pEnabledFeatures        = &enabledFeatures,
    };

    vkhDev = vkh_device_create(app, pi, &device_info);
    presentQIndex = (uint32_t)pi->pQueue;
    renderer =
        vkh_presenter_create(vkhDev, presentQIndex, vkSurf, width, height, VK_FORMAT_B8G8R8A8_SRGB, presentMode);


    vkh_app_free_phyinfos(phyCount, phys);

    glfwSetKeyCallback(win, key_callback);
    glfwSetMouseButtonCallback(win, mouse_button_callback);
    glfwSetCursorPosCallback(win, mouse_move_callback);

}

void SampleApp::Run() {
    uint32_t curIter = 0;

    if (VkvgTest::tests.empty())
        return;

    int testIdx = 0;
    auto it = VkvgTest::tests.begin();
    VkvgTest* curTest = NULL;

    if (testsToRun.empty()) {
        curTest = (*it);
    } else {
        curTest = VkvgTest::tests[testsToRun[testIdx]];
    }

    glfwSetWindowTitle(win, curTest->name.c_str());
    curTest->initTest(this);

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        curTest->performTest();

        if (!vkh_presenter_draw(renderer)) {
            vkh_presenter_get_size(renderer, &width, &height);
            curTest->cleanTest();
            curTest->initTest(this);
            continue;
        }

        if (iterations > 0) {
            if (++curIter == iterations) {
                testIdx++;
                curTest->cleanTest();
                if (testsToRun.empty()) {
                    if (++it == VkvgTest::tests.end()){
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
                glfwSetWindowTitle(win, curTest->name.c_str());
                curTest->initTest(this);
            }
        }
    }
    if (curTest != NULL)
        curTest->cleanTest();
}

void SampleApp::CleanUp() {
    vkDeviceWaitIdle(vkh_device_get_vkdev(vkhDev));
    vkh_presenter_destroy(renderer);
    vkDestroySurfaceKHR(vkh_app_get_inst(app), vkSurf, NULL);

    vkh_device_destroy(vkhDev);
    glfwDestroyWindow(win);
    vkh_app_destroy(app);

    glfwTerminate();
}
