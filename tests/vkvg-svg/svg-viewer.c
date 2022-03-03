#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <stdint.h>

#include <stdarg.h>
#include <ctype.h>

#include "vkvg.h"
#include "vkvg-svg.h"
#include "vkengine.h"

static VkvgDevice dev;
static VkvgSurface svgSurf = NULL;
static double scale = 1;


static char* filename = NULL;
static char* directory = NULL;
static DIR * pCurrentDir = NULL;
struct dirent *dir = NULL;
static int iconSize = -1;
static VkSampleCountFlags samples = VK_SAMPLE_COUNT_8_BIT;
static uint32_t width=512, height=512, margin = 10;
static double scrollX, scrollY;
static bool paused = false, repaintIconList = true;


struct stat file_stat;

#define NORMAL_COLOR  "\x1B[0m"
#define GREEN  "\x1B[32m"
#define BLUE  "\x1B[34m"

static int svg_file_count;
static float maxScroll;

int _count_svg_files () {
	struct dirent *de;
	rewinddir (pCurrentDir);
	int i = 0;
	while ((de = readdir(pCurrentDir)) != NULL) {
		if(de->d_type != DT_DIR && !strcasecmp(strrchr(de->d_name, '\0') - 4, ".svg"))
			i++;
	}
	return i;
}
void readSVG (VkEngine e) {
	struct stat sb;
	VkvgSurface newSvgSurf = NULL;
	if (iconSize > 0 && pCurrentDir) {
		if (!repaintIconList)
			return;
		char tmp[FILENAME_MAX];
		double x = 0, y = 0;
		int cellSize = iconSize + margin;
		int iconPerLine = ceil((double)(width-iconSize) / cellSize);
		int lineToSkip = floor(scrollY / cellSize);
		int iconToSkip = lineToSkip * iconPerLine;

		y = (lineToSkip * cellSize) - scrollY;

		newSvgSurf = vkvg_surface_create(dev, width, height);
		VkvgContext ctx = vkvg_create(newSvgSurf);

		struct dirent *de;
		rewinddir (pCurrentDir);
		int i = 0;
		while ((de = readdir(pCurrentDir)) != NULL) {
			if(de->d_type != DT_DIR) {
				if (!strcasecmp(strrchr(de->d_name, '\0') - 4, ".svg")) {
					if (i >= iconToSkip) {
						sprintf(tmp, "%s%s", directory, de->d_name);
						VkvgSurface surf = vkvg_surface_create_from_svg (dev, iconSize, iconSize, tmp);

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
					i++;
				}
			}
		}
		vkvg_destroy(ctx);
		repaintIconList = false;
	}else if (filename) {
		vkengine_set_title(e, filename);
		if (stat(filename, &sb) == -1) {
			printf ("Unable to stat file: %s\n", filename);
			exit(EXIT_FAILURE);
		}
		if (sb.st_mtim.tv_sec == file_stat.st_mtim.tv_sec)
			return;
		file_stat = sb;
		newSvgSurf = vkvg_surface_create_from_svg(dev, width, height, filename);
	} else if (dir) {
		char tmp[FILENAME_MAX];
		sprintf(tmp, "%s/%s", directory, dir->d_name);
		vkengine_set_title(e, tmp);
		if (stat(tmp, &sb) == -1) {
			printf ("Unable to stat file: %s\n", tmp);
			exit(EXIT_FAILURE);
		}
		if (sb.st_mtim.tv_sec == file_stat.st_mtim.tv_sec)
			return;
		file_stat = sb;
		newSvgSurf = vkvg_surface_create_from_svg(dev, width, height, tmp);
	}

	//vkengine_wait_idle(e);

	if (svgSurf)
		vkvg_surface_destroy(svgSurf);
	svgSurf = newSvgSurf;

	//vkengine_wait_idle(e);
#ifdef VKVG_DBG_STATS
		vkvg_debug_stats_t dbgStats = vkvg_device_get_stats (dev);
		vkvg_device_reset_stats (dev);
		printf("maximum point array size		= %d\n", dbgStats.sizePoints);
		printf("maximum path array size			= %d\n", dbgStats.sizePathes);
		printf("maximum size of host vertice cache	= %d\n", dbgStats.sizeVertices);
		printf("maximum size of host index cache	= %d\n", dbgStats.sizeIndices);
		printf("maximum size of vulkan vertex buffer	= %d\n", dbgStats.sizeVBO);
		printf("maximum size of vulkan index buffer	= %d\n", dbgStats.sizeIBO);
#endif

}

struct dirent *get_next_svg_file_in_current_directory (bool cycle) {
	struct dirent *de;
	while ((de = readdir(pCurrentDir)) != NULL) {
		if(de->d_type != DT_DIR) {
			if (!strcasecmp(strrchr(de->d_name, '\0') - 4, ".svg"))
				return de;
		}
	}
	if (!cycle)
		return NULL;
	rewinddir (pCurrentDir);
	return get_next_svg_file_in_current_directory (false);
}


static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_RELEASE)
		return;
	switch (key) {
	case GLFW_KEY_SPACE:
		 paused = !paused;
		break;
	case GLFW_KEY_R:
		//recording = !recording;
		file_stat = (struct stat){0};
		break;
	case GLFW_KEY_ESCAPE :
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	case GLFW_KEY_ENTER :
		if (!pCurrentDir)
			break;
		dir = get_next_svg_file_in_current_directory(true);
		file_stat = (struct stat){0};
		break;
	case GLFW_KEY_KP_ADD:
		scale*=2.0;
		file_stat = (struct stat){0};
		break;
	case GLFW_KEY_KP_SUBTRACT:
		scale/=2.0;
		file_stat = (struct stat){0};
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
	repaintIconList = true;
}

void print_help_and_exit () {
	printf("\nUsage: svgviewer [options] [svgfilepath]\n\n");
	printf("\t-d directory:\tdirectory containing svg files, cycle pressing Enter.\n");
	printf("\t\t\tif the -d option is not specified, svgfile path is mandatory.\n");
	printf("\t-i size:\tif -d option is present, display all svg files as a list with the size specified.\n");
	printf("\t-m margin:\tset margin for the -i option\n");
	printf("\n");
	exit(-1);
}


int main (int argc, char *argv[]){
	int i = 1;

	while (i < argc) {
		int argLen = strlen(argv[i]);
		if (argv[i][0] == '-') {

			if (argLen < 2)	print_help_and_exit ();

			switch (argv[i][1]) {
			case 'd':
				if (argc < ++i + 1) print_help_and_exit();
				directory = argv[i];
				break;
			case 'w':
				if (argc < ++i + 1) print_help_and_exit();
				width = atoi(argv[i]);
				break;
			case 'h':
				if (argc < ++i + 1) print_help_and_exit();
				height = atoi(argv[i]);
				break;
			case 's':
				if (argc < ++i + 1) print_help_and_exit();
				samples = (VkSampleCountFlags)atoi(argv[i]);
				break;
			case 'i':
				if (argc < ++i + 1) print_help_and_exit();
				iconSize = atoi(argv[i]);
				break;
			case 'm':
				if (argc < ++i + 1) print_help_and_exit();
				margin = atoi(argv[++i]);
				break;
			default:
				print_help_and_exit();
			}
		} else
			filename = argv[i];
		i++;
	}
	if (!filename && !directory)
		print_help_and_exit();

	VkEngine e = vkengine_create (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,VK_PRESENT_MODE_FIFO_KHR, width, height);
	vkengine_set_key_callback (e, key_callback);
	vkengine_set_scroll_callback(e, scroll_callback);

	dev = vkvg_device_create_from_vk_multisample (vkh_app_get_inst(e->app),
			 vkengine_get_physical_device(e), vkengine_get_device(e), vkengine_get_queue_fam_idx(e), 0, samples, false);

	VkvgSurface surf = vkvg_surface_create(dev, width, height);

	vkh_presenter_build_blit_cmd (e->renderer, vkvg_surface_get_vk_image(surf), width, height);

	if (directory) {
		pCurrentDir = opendir(directory);
		if(!pCurrentDir) {
			printf ("Directory not found: %s\n", directory);
			exit(EXIT_FAILURE);
		}
		dir = get_next_svg_file_in_current_directory(false);
		if (!dir) {
			printf ("No .svg file found in %s\n", directory);
			closedir(pCurrentDir);
			exit(EXIT_FAILURE);
		}
		if (iconSize > 0) {
			svg_file_count = _count_svg_files();
			int cellSize = iconSize + margin;
			int iconPerLine = ceil((double)(width-iconSize) / cellSize);
			int visibleLines = ceil((double)(height) / cellSize);
			int totLines = ceil((double)svg_file_count / iconPerLine);
			maxScroll = (totLines-visibleLines) * cellSize;
		}
	}



	while (!vkengine_should_close (e)) {
		//vkvg_log_level = VKVG_LOG_INFO_CMD;
		readSVG (e);
		//vkvg_log_level = VKVG_LOG_ERR;

		VkvgContext ctx = vkvg_create(surf);
		vkvg_set_source_rgb(ctx,0.1,0.1,0.1);
		vkvg_paint(ctx);

		if (svgSurf) {
			vkvg_set_source_surface(ctx, svgSurf, 0, 0);
			vkvg_paint(ctx);
		} else {
			vkvg_set_line_width(ctx,10);
			vkvg_set_source_rgb(ctx,1,0,0);
			vkvg_move_to(ctx, 0,0);
			vkvg_line_to(ctx, 512,512);
			vkvg_move_to(ctx, 0,512);
			vkvg_line_to(ctx, 512,0);
			vkvg_stroke(ctx);

		}
		vkvg_destroy(ctx);

		glfwPollEvents();

		if (!vkh_presenter_draw (e->renderer)){
			vkh_presenter_get_size (e->renderer, &width, &height);
			vkvg_surface_destroy (surf);
			surf = vkvg_surface_create(dev, width, height);
			vkh_presenter_build_blit_cmd (e->renderer, vkvg_surface_get_vk_image(surf), width, height);
			vkengine_wait_idle(e);
			repaintIconList = true;
			continue;
		}

	}
	
	vkengine_wait_idle(e);
	
	if (svgSurf)
		vkvg_surface_destroy(svgSurf);
	vkvg_surface_destroy(surf);
	vkvg_device_destroy(dev);
	vkengine_destroy(e);

	if (pCurrentDir)
		closedir(pCurrentDir);
}
