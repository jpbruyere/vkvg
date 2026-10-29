#pragma once
#include <stdio.h>
#include <vector>
#include <math.h>
#include <assert.h>

#include "vkh.h"
#include "vkvg.h"

#include <GLFW/glfw3.h>
#include <vector>

#define TRY_LOAD_DEVICE_EXT(ext)                                                                                       \
    {                                                                                                                  \
        if (vkh_phyinfo_try_get_extension_properties(pi, #ext, NULL))                                                  \
            enabledExts[enabledExtsCount++] = #ext;                                                                    \
    }
#if defined(__GNUC__) || defined(__clang__)
#define BENCHMARK_ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER) && !defined(__clang__)
#define BENCHMARK_ALWAYS_INLINE __forceinline
#define __func__ __FUNCTION__
#else
#define BENCHMARK_ALWAYS_INLINE
#endif
template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp& value) {
#if defined(__clang__)
    asm volatile("" : "+r,m"(value) : : "memory");
#else
    asm volatile("" : "+m,r"(value) : : "memory");
#endif
}

class VkvgApp {
    bool try_get_phyinfo(VkhPhyInfo* phys, uint32_t phyCount, VkPhysicalDeviceType gpuType, VkhPhyInfo* phy);

  protected:
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        VkvgApp* app = reinterpret_cast<VkvgApp*>(glfwGetWindowUserPointer(window));
        app->OnKey(key, scancode, action, mods);
    }
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        VkvgApp* app = reinterpret_cast<VkvgApp*>(glfwGetWindowUserPointer(window));
        app->OnScroll(xoffset,yoffset);
    }
    static void mouse_move_callback(GLFWwindow* window, double xpos, double ypos) {
        VkvgApp* app = reinterpret_cast<VkvgApp*>(glfwGetWindowUserPointer(window));
        app->OnMouseMove(xpos,ypos);
    }
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int modif) {
        VkvgApp* app = reinterpret_cast<VkvgApp*>(glfwGetWindowUserPointer(window));
        app->OnMouseButton(button,action,modif);
    }

    VkPhysicalDeviceMemoryProperties memory_properties;
    VkPhysicalDeviceProperties       gpu_props;

  public:

    uint32_t              width       = 1024;
    uint32_t              height      = 1024;
    std::vector<uint32_t> logSeverity = {1};
    uint32_t              logType     = 0;
    bool                  listGpus    = false;
    bool                  save_img    = false;
    int32_t               gpuIndex    = -1;
    VkSampleCountFlags    samples     = VK_SAMPLE_COUNT_1_BIT;

    VkPhysicalDeviceType  preferedGPU = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    VkPresentModeKHR      presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

    VkvgDevice   device;
    VkvgSurface  surf;

    GLFWwindow*  win;
    VkSurfaceKHR vkSurf;
    VkhApp       app;
    VkhDevice    vkhDev;
    VkhPresenter renderer;
    uint32_t     presentQIndex;
    bool         offscreen;
    bool         displayMode;
    bool         refresh = true;

    double zoom = 1.0;
    double offsetX = 0.0;
    double offsetY = 0.0;

    bool isDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    virtual void Init();
    virtual void CleanUp();
    virtual void Render();

    virtual void OnKey (int key, int scancode, int action, int mods);
    virtual void OnScroll (double xoffset, double yoffset);
    virtual void OnMouseButton (int button, int action, int mods);
    virtual void OnMouseMove (double xpos, double ypos);

    void Run();

};
