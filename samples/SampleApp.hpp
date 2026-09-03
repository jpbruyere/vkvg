#pragma once
#include <stdio.h>
#include <vector>
#include <math.h>
#include <assert.h>

#include "vkh.h"
#include "vkvg.h"

#include <GLFW/glfw3.h>

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

class SampleApp {
    bool try_get_phyinfo(VkhPhyInfo* phys, uint32_t phyCount, VkPhysicalDeviceType gpuType, VkhPhyInfo* phy);

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    VkPhysicalDeviceMemoryProperties memory_properties;
    VkPhysicalDeviceProperties       gpu_props;

  public:
    uint32_t              width       = 512;
    uint32_t              height      = 512;
    uint32_t              iterations  = 0;
    uint32_t              testSize    = 500;
    std::vector<uint32_t> logSeverity = {1};
    uint32_t              logType     = 0;
    bool                  listGpus    = false;
    bool                  save_img    = false;
    int32_t               gpuIndex    = -1;
    VkSampleCountFlags    samples     = VK_SAMPLE_COUNT_1_BIT;

    std::vector<uint32_t> testsToRun;
    VkPhysicalDeviceType  preferedGPU = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    VkPresentModeKHR      presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

    GLFWwindow*  win;
    VkSurfaceKHR vkSurf;
    VkhApp       app;
    VkhDevice    vkhDev;
    VkhPresenter renderer;
    uint32_t     presentQIndex;

    void Init();
    void Run();
    void CleanUp();
};
