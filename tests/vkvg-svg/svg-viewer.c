#include "vkvg.h"
#include "vkvg-svg.h"
#include "vkengine.h"
#include "cross_os.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <stdint.h>

#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

static VkvgDevice  dev;
static VkvgSurface mainSurf = NULL;
static VkEngine    e       = NULL;
static double      scale   = 1;

static char*              path        = NULL;
static char*              output      = NULL;
static uint32_t           log_level   = VKVG_LOG_ERR; // | VKVG_LOG_INFO | VKVG_LOG_DEBUG | VKVG_LOG_INFO_PTS; VKVG_LOG_INFO_VBO;

static bool               recurse     = false;
static int                iconSize    = -1;
static VkSampleCountFlags samples     = VK_SAMPLE_COUNT_8_BIT;
static uint32_t           width = 512, height = 512, margin = 10;
static double             scrollX, scrollY;
static bool               paused = false, update = true, nextSvgFile = true;

struct stat file_stat;

#define BACKGROUND_COLOR 0.7,0.7,0.8
#define MAX_PATH_LENGTH 1024

#define NORMAL_COLOR "\x1B[0m"
#define GREEN        "\x1B[32m"
#define BLUE         "\x1B[34m"

static int    svg_file_count = 0;

static float  maxScroll    = 0;
static int    cellSize     = 0;
static int    iconPerLine  = 0;
static int    lineToSkip   = 0;
static int    iconToSkip   = 0;
static int    visibleLines = 0;
static int    totLines     = 0;
static int    skipedIcons  = 0;
static int    drawnIcons   = 0;

/* not defined in c11, ok to define here only for sample */
#ifndef DT_DIR
#define DT_DIR 4
#endif




uint32_t count_svg_files(DIR* pDir) {
    rewinddir(pDir);
    struct dirent* de;
    uint32_t count = 0;
    char path[MAX_PATH_LENGTH];
    while ((de = readdir(pDir)) != NULL) {
        if (de->d_type != DT_DIR && !strcasecmp(strrchr(de->d_name, '\0') - 4, ".svg"))
            count++;
    }
    rewinddir(pDir);
    return count;
}

static inline void queryUpdate() {
    update = true;
    lineToSkip = floor(scrollY / cellSize);
    iconToSkip  = lineToSkip * iconPerLine;
    file_stat = (struct stat){0};
}

struct dirent* get_next_svg_file_in_dir(DIR* pDir, bool cycle) {
    struct dirent* de;
    while ((de = readdir(pDir)) != NULL) {
        if (de->d_type != DT_DIR) {
            if (!strcasecmp(strrchr(de->d_name, '\0') - 4, ".svg"))
                return de;
        }
    }
    if (!cycle)
        return NULL;
    rewinddir(pDir);
    return get_next_svg_file_in_dir(pDir, false);
}


static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_RELEASE)
        return;
    switch (key) {
    case GLFW_KEY_SPACE:
        paused = !paused;
        break;
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    case GLFW_KEY_ENTER:
        nextSvgFile = true;
        queryUpdate();
        break;
    case GLFW_KEY_KP_ADD:
        scale *= 2.0;
        queryUpdate();
        break;
    case GLFW_KEY_KP_SUBTRACT:
        scale /= 2.0;
        queryUpdate();
        break;
    }
}
static void scroll_callback(GLFWwindow* window, double x, double y) {
    if (iconSize == 0)
        return;
    scrollX -= x * 25;
    scrollY -= y * 25;
    if (scrollX < 0)
        scrollX = 0;
    if (scrollY < 0)
        scrollY = 0;
    else if (scrollY > maxScroll)
        scrollY = maxScroll;
    queryUpdate();
}

void print_help_and_exit() {
    printf("\nUsage: svgviewer [options] [path]\n");
    printf("\tIf path is a directory and '-i' option is absent, you may cycle files pressing enter.\n");
    printf("\t-o file.png:\toutput result to file then exit.\n");
    printf("\t-r recurse sub-dir if path is a directory\n");
    //printf("\t-d directory:\tdirectory containing svg files, cycle pressing Enter.\n");
    //printf("\t\t\tif the -d option is not specified, svgfile path is mandatory.\n");
    printf("\t-i size:\tif path is a directory, display all svg files as an icon list with the size specified.\n");
    printf("\t-m margin:\tset margin for the -i option\n");
    printf("\t-w width:\tset output surface width.\n");
    printf("\t-h height:\tset output surface height.\n");
    printf("\t-s samples:\tset sample count, set to 1 to disable multisampling.\n");
    //printf("\t-r record only:\tload and emit drawing commands without performing draw (parser tests).\n\n");
    printf("\n");
    exit(-1);
}

int main(int argc, char* argv[]) {
    int   i      = 1;
    bool record = false;

    while (i < argc) {
        int argLen = strlen(argv[i]);
        if (argv[i][0] == '-') {

            if (argLen < 2)
                print_help_and_exit();

            switch (argv[i][1]) {
            case 'w':
                if (argc < ++i + 1)
                    print_help_and_exit();
                width = atoi(argv[i]);
                break;
            case 'h':
                if (argc < ++i + 1)
                    print_help_and_exit();
                height = atoi(argv[i]);
                break;
            case 's':
                if (argc < ++i + 1)
                    print_help_and_exit();
                samples = (VkSampleCountFlags)atoi(argv[i]);
                break;
            case 'i':
                if (argc < ++i + 1)
                    print_help_and_exit();
                iconSize = atoi(argv[i]);
                break;
            case 'm':
                if (argc < ++i + 1)
                    print_help_and_exit();
                margin = atoi(argv[i]);
                break;
            case 'o':
                if (argc < ++i + 1)
                    print_help_and_exit();
                output = argv[i];
                break;
            /*case 'r':
                record = true;*/
            default:
                print_help_and_exit();
            }
        } else
            path = argv[i];
        i++;
    }
    if (!path)
        print_help_and_exit();

    //vkh_log_level = VKVG_LOG_INFO;

    DIR* pCurrentDir = NULL;
    struct stat sb;
    if (stat(path, &sb) == -1) {
        printf("Unable to stat path: %s\n", path);
        exit(EXIT_FAILURE);
    }

    if (S_ISDIR(sb.st_mode)) {
        if (output) {
            if (!strcasecmp(strrchr(output, '\0') - 4, ".png")) {
                printf("If svg path is a directory, output must also be a directory\n");
                exit(EXIT_FAILURE);
            }
            if (!create_dir_tree(output)) {
                printf("Error creating directory %s\n", output);
                exit(EXIT_FAILURE);
            }
        }

        pCurrentDir = opendir(path);
        if (!pCurrentDir) {
            printf("Error opening directory: %s\n", path);
            exit(EXIT_FAILURE);
        }

        svg_file_count = count_svg_files(pCurrentDir);

        if (!svg_file_count) {
            printf("No .svg file found in %s\n", path);
            closedir(pCurrentDir);
            exit(EXIT_FAILURE);
        }

        if (output) {
            vkvg_device_create_info_t info = {0};
            dev = vkvg_device_create(&info);
            VkvgSurface svgSurf = NULL;

            struct dirent* de = get_next_svg_file_in_dir(pCurrentDir, false);
            while(de) {
                char tmp[MAX_PATH_LENGTH];

                sprintf(tmp, "%s/%s", path, de->d_name);
                svgSurf = vkvg_surface_create_from_svg(dev, width, height, tmp);

                sprintf(tmp, "%s/%.*s.png", output, (int)(strlen(de->d_name) - 4), de->d_name);
                vkvg_surface_write_to_png(svgSurf, tmp);

                vkvg_surface_destroy(svgSurf);
                de = get_next_svg_file_in_dir(pCurrentDir, false);
            }
            vkvg_device_destroy(dev);
            closedir(pCurrentDir);
            exit(EXIT_SUCCESS);
        }

        if (iconSize > 0) {
            cellSize     = iconSize + margin;
            iconPerLine  = ceil((double)(width - iconSize) / cellSize);
            visibleLines = ceil((double)(height) / cellSize);
            totLines     = ceil((double)svg_file_count / iconPerLine);
            maxScroll    = (totLines - visibleLines) * cellSize;
        }
    } else if (output) {
        vkvg_device_create_info_t info = {0};
        dev = vkvg_device_create(&info);
        VkvgSurface svgSurf = vkvg_surface_create_from_svg(dev, width, height, path);
        vkvg_surface_write_to_png(svgSurf, output);
        vkvg_surface_destroy(svgSurf);
        vkvg_surface_destroy(svgSurf);
        vkvg_device_destroy(dev);
    }

    VkEngine e = vkengine_create(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PRESENT_MODE_FIFO_KHR, width, height);
    vkengine_set_key_callback(e, key_callback);
    vkengine_set_scroll_callback(e, scroll_callback);
    vkvg_device_create_info_t info = {samples,
                                      false,
                                      vkh_app_get_inst(e->app),
                                      vkengine_get_physical_device(e),
                                      vkengine_get_device(e),
                                      vkengine_get_queue_fam_idx(e),
                                      0};
    dev                            = vkvg_device_create(&info);
    mainSurf = vkvg_surface_create(dev, width, height);
    vkh_presenter_build_blit_cmd(e->renderer, vkvg_surface_get_vk_image(mainSurf), width, height);


    char   tmp[FILENAME_MAX];
    struct dirent* de = NULL;
    while (!vkengine_should_close(e)) {

        glfwPollEvents();

        if (iconSize > 0 && pCurrentDir) {
            if (update) {
                queryUpdate();
                vkengine_set_title(e, path);
                double x = 0;
                double y = (lineToSkip * cellSize) - scrollY;

                VkvgContext ctx = vkvg_create(mainSurf);
                vkvg_set_source_rgb(ctx, BACKGROUND_COLOR);
                vkvg_paint(ctx);

                rewinddir(pCurrentDir);
                int i = 0;
                de = get_next_svg_file_in_dir(pCurrentDir, false);
                while (de) {
                    if (i >= iconToSkip) {
                        sprintf(tmp, "%s/%s", path, de->d_name);
                        VkvgSurface surf = vkvg_surface_create_from_svg(dev, iconSize, iconSize, tmp);
                        if (surf) {
                            vkvg_set_source_surface(ctx, surf, x, y);
                            vkvg_paint(ctx);
                            vkvg_surface_destroy(surf);
                        }
                        x += iconSize + margin;
                        if (x > width - iconSize) {
                            x = 0;
                            y += iconSize + margin;
                            if (y >= height)
                                break;
                        }
                    }
                    de = get_next_svg_file_in_dir(pCurrentDir, false);
                    i++;
                }
                vkvg_destroy(ctx);
                update = false;
            }
        } else {
            if (pCurrentDir) {
                if (!de || nextSvgFile) {
                    de = get_next_svg_file_in_dir(pCurrentDir, true);
                    nextSvgFile = false;
                }
                sprintf(tmp, "%s/%s", path, de->d_name);
            } else {
                sprintf(tmp, "%s", path);
            }
            if (stat(tmp, &sb) == -1) {
                printf("Unable to stat file: %s\n", tmp);
                exit(EXIT_FAILURE);
            }
            if (update || sb.st_mtime != file_stat.st_mtime) {
                vkengine_set_title(e, path);
                file_stat  = sb;
                VkvgSurface surf = vkvg_surface_create_from_svg(dev, width, height, tmp);
                VkvgContext ctx = vkvg_create(mainSurf);
                vkvg_set_source_rgb(ctx, BACKGROUND_COLOR);
                vkvg_paint(ctx);
                vkvg_set_source_surface(ctx, surf, 0, 0);
                vkvg_paint(ctx);
                vkvg_surface_destroy(surf);
                vkvg_destroy(ctx);
                update = false;
            }
        }

        if (!vkh_presenter_draw(e->renderer)) {
            vkh_presenter_get_size(e->renderer, &width, &height);
            vkvg_surface_destroy(mainSurf);
            mainSurf = vkvg_surface_create(dev, width, height);
            vkh_presenter_build_blit_cmd(e->renderer, vkvg_surface_get_vk_image(mainSurf), width, height);
            vkengine_wait_idle(e);
            queryUpdate();
            continue;
        }

    }

    if (pCurrentDir)
        closedir(pCurrentDir);

    vkengine_wait_idle(e);
    vkvg_surface_destroy(mainSurf);
    vkvg_device_destroy(dev);
    vkengine_destroy(e);

    exit(0);
}
