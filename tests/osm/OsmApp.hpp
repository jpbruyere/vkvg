#pragma once
#include <stdio.h>
#include <vector>
#include <math.h>
#include <assert.h>

#include "vkh.h"
#include "vkvg.h"

#include <GLFW/glfw3.h>
#include <vector>
#include <osmium/geom/coordinates.hpp>

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

enum class RoadType { Primary, Secondary, Tertiary, Residential, Service, Unknown };

struct MapWay {
    std::vector<osmium::geom::Coordinates> points;
    std::string name;
    RoadType road_type = RoadType::Unknown;
    bool is_building = false;
    bool is_water = false;
};

struct MapNode {
    osmium::geom::Coordinates coord;
    std::string name;
    std::string type; // e.g., "restaurant", "cafe", "tree"
};

class OsmApp {
    bool try_get_phyinfo(VkhPhyInfo* phys, uint32_t phyCount, VkPhysicalDeviceType gpuType, VkhPhyInfo* phy);

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);


    VkPhysicalDeviceMemoryProperties memory_properties;
    VkPhysicalDeviceProperties       gpu_props;

  public:
    uint32_t              width       = 512;
    uint32_t              height      = 512;
    std::vector<uint32_t> logSeverity = {1};
    uint32_t              logType     = 0;
    bool                  listGpus    = false;
    bool                  save_img    = false;
    int32_t               gpuIndex    = -1;
    VkSampleCountFlags    samples     = VK_SAMPLE_COUNT_1_BIT;

    VkPhysicalDeviceType  preferedGPU = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    VkPresentModeKHR      presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

    GLFWwindow*  win;
    VkSurfaceKHR vkSurf;
    VkhApp       app;
    VkhDevice    vkhDev;
    VkhPresenter renderer;
    uint32_t     presentQIndex;
    bool         offscreen;
    bool         displayMode;
    bool         refresh = true;

    // Navigation Parameters
    double zoom = 1.0;
    double offsetX = 0.0;
    double offsetY = 0.0;

    // Mouse state trackers
    bool isDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    // Geographic Base Extents
    double min_merc_x = 0.0;
    double max_merc_y = 0.0;
    double base_scale = 1.0;

    std::vector<MapWay> parsed_ways;
    std::vector<MapNode> parsed_nodes;

    VkvgDevice  device;
    VkvgSurface surf;

    void Init();
    void Run();
    void CleanUp();
    void Render();
    void render_svg_icon(VkvgContext ctx, const std::string& type, double x, double y);
};
