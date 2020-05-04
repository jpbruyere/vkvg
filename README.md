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
</p>
</h1>

**vkvg** is an open source *2D graphics library* written in **c**  using [Vulkan](https://www.khronos.org/vulkan/) as backend. It's **api** follows the same pattern as [Cairo](https://www.cairographics.org/), but new functions and original drawing mechanics may be added.

**vkvg** is in early development stage, api may change, any contribution is welcome.

For API documentation and usage, please refer to the [Cairo](https://www.cairographics.org/) documentation for now.

### Current status:

- Fill (stencil even-odd, non-zero with ear clipping).
- Stroke.
- Basic painting operation.
- Font system with caching operational.
- Linear Gradients.
- Line caps and joins.
- Context should be thread safe, extensive tests required.
- Image loading and writing with [stb lib](https://github.com/nothings/stb)
- Test includes svg rendering with [nanoSVG](https://github.com/memononen/nanosvg)

<p align="center">
  <a href="https://github.com/jpbruyere/vkvg/blob/master/vkvg-tiger.png">
	<kbd><img src="https://github.com/jpbruyere/vkvg/blob/master/vkvg-tiger.png" height="260"></kbd>
  </a>
  <a href="https://github.com/jpbruyere/vkvg/blob/master/screenshot1.png">
	<kbd><img src="https://github.com/jpbruyere/vkvg/blob/master/screenshot1.png" height="260"></kbd>
  </a>
</p>

### Requirements:

- [CMake](https://cmake.org/): version > 12.
- [Vulkan](https://www.khronos.org/vulkan/)
- [FontConfig](https://www.freedesktop.org/wiki/Software/fontconfig/)
- [Freetype](https://www.freetype.org/)
- [Harfbuzz](https://www.freedesktop.org/wiki/Software/HarfBuzz/)
- GLSLC: spirv compiler, included in [LunarG SDK](https://www.lunarg.com/vulkan-sdk/) (building only)
- [xxd](https://linux.die.net/man/1/xxd): generate headers with precompiled shaders (building only)
- [GLFW](http://www.glfw.org/): optional, if present tests are built.

if `glslc` or `xxd` are not present, a precompiled version of the shaders is stored in the git tree.

### Building

The library can be built on [Windows](doc/windows_build_tutorial.md) or on [Linux](doc/linux_build_tutorial.md) throught CMake. 

### To Do

- [x] Use Scissor where possible.
- [x] Improve stroke algorithms.
- [ ] Radial gradients.
- [ ] Dashed lines.
- [ ] Operators.
- [x] Optimize vulkan memory allocations by sub-allocating from a single shared memory chunk per type.
- [x] Optimize command submissions.
- [x] Test SDF font rendering.
- [x] Avoid line joins inside curves and arc.
- [ ] Structured unit testing.
- [ ] Perf and memory checks.
- [ ] Code clean and comment.
- [ ] Documentations.


