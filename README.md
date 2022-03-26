<h1 align="center">
  <br>
  <a href="https://github.com/jpbruyere/vkvg/blob/master/vkvg.svg">
	<img src="https://github.com/jpbruyere/vkvg/blob/master/vkvg.svg?sanitize=true" alt="vkvg" width="140">
  </a>
  <br>
	<br>
  Vulkan Vector Graphics
  <br>
<p align="center">
  <a href="https://github.com/jpbruyere/vkvg/actions/workflows/cmake.yml">
     <img src="https://github.com/jpbruyere/vkvg/actions/workflows/cmake.yml/badge.svg">
  </a>
  <a href="https://travis-ci.org/jpbruyere/vkvg">
    <img src="https://img.shields.io/travis/jpbruyere/vkvg.svg?label=Linux&logo=travis&logoColor=white&message=build">
  </a>
  <a href="https://ci.appveyor.com/project/jpbruyere/vkvg">
    <img src="https://img.shields.io/appveyor/ci/jpbruyere/vkvg?label=Win64&logo=appveyor&logoColor=lightgrey">
  </a>
  <img src="https://img.shields.io/github/license/jpbruyere/vkvg.svg?style=flat-square">
  <a href="https://www.paypal.me/GrandTetraSoftware">
    <img src="https://img.shields.io/badge/Donate-PayPal-blue.svg?style=flat-square">
  </a>
  <a href="https://gitter.im/CSharpRapidOpenWidgets?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge">
    <img src="https://badges.gitter.im/CSharpRapidOpenWidgets.svg">
  </a>
</p>
<p align="center">
  <a href="https://aur.archlinux.org/packages/vkvg">
    <img src="https://img.shields.io/aur/version/vkvg">
  </a>
  <a href="https://mpr.hunterwittenborn.com/packages/vkvg">
    <img src="https://img.shields.io/badge/mpr-v0.3.0--beta-blue">
  </a>
</p>
	
</h1>



**vkvg** is an open-source *2D graphics library* written in **C** using [Vulkan](https://www.khronos.org/vulkan/) as backend. The **API** follows the same pattern as [Cairo](https://www.cairographics.org/), but new functions and original drawing mechanics may be added.

**vkvg** is in early development stage, the API is subject to change. All contributions are welcome.

For API documentation and usage, please refer to the [Cairo](https://www.cairographics.org/) for now.

<p align="center">
  <a href="https://github.com/jpbruyere/vkvg/blob/master/vkvg-tiger.png">
	<kbd><img src="https://github.com/jpbruyere/vkvg/blob/master/vkvg-tiger.png" height="260"></kbd>
  </a>
  <a href="https://github.com/jpbruyere/vkvg/blob/master/screenshot3.png">
	<kbd><img src="https://raw.githubusercontent.com/jpbruyere/vkvg/master/screenshot3.png" height="260"></kbd>
  </a>
  <a href="https://github.com/jpbruyere/vkvg/blob/master/screenshot1.png">
	<kbd><img src="https://github.com/jpbruyere/vkvg/blob/master/screenshot1.png" height="260"></kbd>
  </a>
</p>

## Performance comparison

<p align="center">
  <a href="https://github.com/jpbruyere/vgperf/blob/master/vgperf.png">
	<kbd><img src="https://raw.githubusercontent.com/jpbruyere/vgperf/master/vgperf.png" height="300"></kbd>
  </a>
   <br>major libs perf comparison</br>
</p>

## Current status:

- Fill (stencil even-odd, non-zero).
- Stroke.
- Basic painting operation.
- Font system with caching operational.
- Linear/Radial Gradients.
- Line caps and joins.
- Dashes.
- Context should be thread safe, extensive tests required.
- Image loading and writing with [stb lib](https://github.com/nothings/stb)
- Test includes svg rendering either with built-in renderer or [nanoSVG](https://github.com/memononen/nanosvg)

## Requirements:

- [CMake](https://cmake.org/): version >= 3.16
- [Meson](https://mesonbuild.com): version >= 0.62.0
- [Vulkan](https://www.khronos.org/vulkan/): version >= 1.1
- [FontConfig](https://www.freedesktop.org/wiki/Software/fontconfig/): optional, without fontconfig, use `vkvg_load_font_from_path`.
- [Freetype](https://www.freetype.org/): optional, stb_truetype as alternative.
- [Harfbuzz](https://www.freedesktop.org/wiki/Software/HarfBuzz/): optional, without complex text shaping may be wrong.
- GLSLC: spirv compiler, included in [LunarG SDK](https://www.lunarg.com/vulkan-sdk/): compile shader to spir-V (building only, optional)
- [xxd](https://linux.die.net/man/1/xxd): generate headers with precompiled shaders (building only, optional)
- [GLFW](http://www.glfw.org/): optional, if present tests are built.

if `glslc` or `xxd` are not present, a precompiled version of the shaders is stored in the git tree.

## Building

<a href="https://aur.archlinux.org/packages/vkvg"><img src="https://img.shields.io/aur/version/vkvg"></a>
<a href="https://mpr.hunterwittenborn.com/packages/vkvg"><img src="https://img.shields.io/badge/mpr-v0.3.0--beta-blue"></a>

## Meson Build
```bash
# fetch sources from github
git clone --recursive https://github.com/jpbruyere/vkvg.git
cd vkvg
# set up and configure meson
meson setup build
# View available options to configure
meson configure build
# Set up install directory
mkdir install
meson configure build -Dprefix=$(pwd)/install
# If you wish to install into /usr/local prefix then you can do this instead
meson configure build -Dprefix=/usr/local
# Build vkvg project
meson compile -C build
# Install vkvg project
meson install -C build
```
### Meson configure options

| Option Name | Type of Value | Default Value | Description |
| ----------- | ------------- | ------------- | ----------- |
| ENABLE_VALIDATION_OPT | boolean | false | Enable Vulkan Validation Layer |
| VKH_ENABLE_VMA | boolean | false | Enable Vulkan Memory Allocator - For more information: https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator |
| VMA_RECORDING_ENABLED | boolean | false | Enable VMA memory recording for debugging |
| VMA_USE_STL_CONTAINERS | boolean | false | Use C++ STL containers instead of VMAs containers |
| VMA_STATIC_VULKAN_FUNCTIONS | boolean | false | Link statically with Vulkan API |
| VMA_DYNAMIC_VULKAN_FUNCTIONS | boolean | true | Fetch pointers to Vulkan functions internally (no static linking) |
| VMA_DEBUG_ALWAYS_DEDICATED_MEMORY | boolean | false | Every allocation will have its own memory block |
| VMA_DEBUG_INITIALIZE_ALLOCATIONS | boolean | false | Automatically fill new allocations and destroyed allocations with some bit pattern |
| VMA_DEBUG_GLOBAL_MUTEX | boolean | false | Enable single mutex protecting all entry calls to the library |
| VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT | boolean | false | Never exceed VkPhysicalDeviceLimits::maxMemoryAllocationCount and return error |
| VMA_DEBUG_ALIGNMENT | integer | 1 | Minimum alignment of all allocation |
| VMA_DEBUG_MARGIN | integer | 0 | Minimum margin before and after every allocation |
| VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY | integer | 1 | Minimum value for VkPhysicalDeviceLimits::bufferImageGranularity. Set to more than 1 for debugging purposes only. Must be power of two. |
| VMA_SMALL_HEAP_MAX_SIZE | integer | 1073741824 | Maximum size of a memory heap in Vulkan to consider it "small". Default is 1 Gigabyte. |
| VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE | integer | 268435456 | Default size of a block allocated as single VkDeviceMemory from a "large" heap. Default is 256 Megabytes. |
| VKVG_ENABLE_DBG_UTILS | boolean | false | Enable VKVG Debug Utilities |
| VKVG_ENABLE_PROFILING | boolean | false | Compile with -pg options |
| VKVG_RECORDING | boolean | true | Enable experimental recording functions |
| VKVG_PREMULT_ALPHA | boolean | true | Use premultiplied alpha for internal rendering |
| VKVG_DBG_STATS | boolean | true | Record contexts statistics in the device |
| VKVG_USE_GLUTESS | boolean | true | Fill non-zero with glu tesselator |
| VKVG_USE_FREETYPE | boolean | true | Use freetype to load and render font glyphs |
| VKVG_USE_FONTCONFIG | boolean | true | Use FontConfig to resolve font names |
| VKVG_USE_HARFBUZZ | boolean | true | Use harfbuzz for text layouting |
| VKVG_LCD_FONT_FILTER | boolean | false | Enable freetype lcd font filtering |
| VKVG_VK_SCALAR_BLOCK_SUPPORTED | boolean | true | Enable scalar block layout support |
| VKVG_ENABLE_VALIDATION | boolean | false | Apply validation |
| VKVG_ENABLE_RENDERDOC | boolean | false | Add vulkan layers for supporting RenderDoc |
| VKVG_ENABLE_WIRED_FILL | boolean | false | Enable Debug Wire Fill |
| ENABLE_TEST | boolean | true | Determines if test cases should be built |
| INSTALL_TEST | boolean | false | Determines if test cases should be installed |
| TEST_HIDE_WARNINGS | boolean | true | Hide all warnings shown in Test Cases compilation |
| COMPILE_SHADERS | boolean | false | Determines if shaders should be recompiled and have shaders.h generated. (Note that shaders.h must be copied over to src/ folder as Meson Build does not allows in-source modification during build) |
| ANDROID_NDK | string |  | Path to Android NDK Directory (This is used to find glslc program for shader compilation) |
| VULKAN_SDK | string |  | Path to Vulkan SDK Directory (This is used to find glslc program for shader compilation) |
| GLSLC_PATH | string |  | Specify absolute path to glslc if it cannot be found by any other means (This is used to find glslc program for shader compilation) |
| XXD_PATH | string |  | Specify absolute path to xxd if it cannot be found by any other means (This is used to find glslc program for shader compilation) |

## Running Meson Tests

```bash
cd ./install/bin/vkvg_tests/
./arcs_test
# Or run any other test found in this directory
ls -l
```

#### Future Note
`meson test` cases are work in progress.

## CMake
```bash
# fetch sources from github
git clone --recursive https://github.com/jpbruyere/vkvg.git
cd vkvg
# Create build directory
mkdir build
cd build
# Run CMake configuration
cmake ..
```

### CMake configure options

##### Core library options:

* `-DVKVG_USE_GLUTESS=true`: Use embedded glu tesselator to fill polygones in NON-ZERO mode. If false, a simple ear clipping algorithm is used.
* `-DVKVG_SVG=true`: Enable experimental svg renderer. If false, use nanoSVG.
* `-DVKVG_RECORDING=true`: Enable experimental draw commands recording infrastructure.
* `-DVKVG_BUILD_DOCS=true`: Build documentation if doxygen is found.

##### Text rendering libraries:

Those libraries are enabled by default, but disabled if not found.
* `-DVKVG_USE_FONTCONFIG=true`: enable FontConfig to resolve font's names.
* `-DVKVG_USE_FREETYPE=true`: enable FreeType to render glyphs, if false glyphs are rendered with stb_truetype.
* `-DVKVG_USE_HARFBUZZ=true`: enable harfbuzz for text shaping.

##### Tests options:

* `-DVKVG_BUILD_TESTS=true`: build all tests in the tests forlder.
* `-DVKVG_TEST_DIRECT_DRAW=true`: enable drawing directly on the swapchain images.

##### Debugging options:

If vkvg is compiled with `CMAKE_BUILD_TYPE=Debug`, several additional options are made available to help debugging:
* `-DENABLE_VALIDATION=true`: enable vulkan validation layer.
* `-DENABLE_DBG_UTILS=true`: enable various vulkan debug utils extensions features.
* `-DENABLE_RENDERDOC=true`: enable renderdoc layer.
* `-DENABLE_WIRED_FILL=true`: enable rendering in wired mode, current mode is controled with the global variable `vkvg_wired_debug`.
* `-DENABLE_PROFILING=true`: add -pg to the compile options.
* `-DVKVG_DBG_STATS=true`: store various context statistics fetchable with `vkvg_device_get_stats()`

```bash
cmake --build .
```
A [detailed tutorial](doc/windows_build_tutorial.md) is available for Windows.

## Running tests

Append the `-h` option to see available command line parameters.

## Contributing

See the [contribution guide](https://github.com/jpbruyere/vkvg/blob/master/CONTRIBUTING.md) for more information.

Join us on [gitter](https://gitter.im/CSharpRapidOpenWidgets) for any question.

## Addtitional Credits

- vkvg use the [AGG project](http://antigrain.com/)'s recursive bezier algorithm which is kindly exposed [here](http://agg.sourceforge.net/antigrain.com/research/adaptive_bezier/index.html).
- I've learned Vulkan with the help of the excellent [Sacha Willems's vulkan examples](https://github.com/SaschaWillems/Vulkan).
- The 2d affine matrix implementation follows Cairo's one.

## Change log

* v0.2.0
	- radial gradients.
	- better stroke joins handling.
	- png saved in srgb format.
	- doxygen cmake target and style

## To Do

- [x] Use Scissor where possible.
- [x] Improve stroke algorithms.
- [x] Radial gradients.
- [x] Dashed lines.
- [ ] Operators.
- [x] Optimize vulkan memory allocations by sub-allocating from a single shared memory chunk per type.
- [x] Optimize command submissions.
- [x] Test SDF font rendering.
- [x] Avoid line joins inside curves and arc.
- [ ] Structured unit testing.
- [x] Perf and memory checks.
- [ ] Code cleanup and comments.
- [ ] Documentations.
