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

- Fill (stencil even-odd, non-zero with ear clipping).
- Stroke.
- Basic painting operation.
- Font system with caching operational.
- Linear Gradients.
- Line caps and joins.
- Context should be thread safe, extensive tests required.
- Image loading and writing with [stb lib](https://github.com/nothings/stb)
- Test includes svg rendering with [nanoSVG](https://github.com/memononen/nanosvg)

## Requirements:

- [CMake](https://cmake.org/): version >= 3.16
- [Vulkan](https://www.khronos.org/vulkan/)
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

```bash
#fetch sources from github
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
* `-DVKVG_USE_FREETYPE=true`: enable FreeType to render glyphs, if false glyphs are rendered with stb_freetype.
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
