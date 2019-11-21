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
	<img src="https://travis-ci.org/jpbruyere/vkvg.svg?branch=master">
  </a>
  <img src="https://img.shields.io/github/license/jpbruyere/vkvg.svg?style=flat-square">
  <a href="https://www.paypal.me/GrandTetraSoftware">
	<img src="https://img.shields.io/badge/Donate-PayPal-blue.svg?style=flat-square">
  </a>
</p>
</h1>

### What is vkvg?
**vkvg** is a multiplatform **c** library for drawing 2D vector graphics with [Vulkan](https://www.khronos.org/vulkan/). It's api follows the same pattern as [Cairo](https://www.cairographics.org/), but new functions and original drawing mechanics may be added.

**vkvg** is in early development stage, api may change, any contribution is welcome.

For API documentation and usage, please refer to the [Cairo](https://www.cairographics.org/) documentation.

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

### Performances:

Resolution  = 512 x 512
Tiling = Optimal

--------------------------------------------------------------------------------------------
| Test Name       |    Library      |   Min    |   Max    |  Average |  Median  | Std Deriv|
|-----------------|-----------------|----------|----------|----------|----------|----------|
| lines stroke    | vkvg            | 0.000585 | 0.007052 | 0.001050 | 0.000711 | 0.000924 |
|                 | cairo xcb       | 0.020720 | 0.032047 | 0.022706 | 0.022320 | 0.001714 |
|                 | cairo es3       | 0.014067 | 0.020243 | 0.015170 | 0.014799 | 0.001199 |
|                 | skia            | 0.000997 | 0.007187 | 0.001606 | 0.001172 | 0.000960 |
| rectangles      | vkvg            | 0.003589 | 0.007038 | 0.004500 | 0.004543 | 0.000421 |
|                 | cairo xcb       | 0.000278 | 0.027525 | 0.002407 | 0.001786 | 0.003300 |
|                 | cairo es3       | 0.001949 | 0.006981 | 0.006748 | 0.006822 | 0.000578 |
|                 | skia            | 0.001894 | 0.003746 | 0.002536 | 0.002437 | 0.000310 |
| circles         | vkvg            | 0.005377 | 0.015504 | 0.006569 | 0.005878 | 0.001892 |
|                 | cairo xcb       | 0.039186 | 0.048615 | 0.041653 | 0.041591 | 0.001257 |
|                 | cairo es3       | 0.028110 | 0.057388 | 0.029743 | 0.029433 | 0.002821 |
|                 | skia            | 0.006685 | 0.009883 | 0.008552 | 0.008832 | 0.000768 |
| stars           | vkvg            | 0.001048 | 0.001652 | 0.001172 | 0.001134 | 0.000118 |
|                 | cairo xcb       | 0.011476 | 0.019959 | 0.015441 | 0.015245 | 0.001388 |
|                 | cairo es3       | 0.009518 | 0.026849 | 0.010171 | 0.009951 | 0.001730 |
|                 | skia            | 0.020135 | 0.046562 | 0.023530 | 0.023253 | 0.002584 |
--------------------------------------------------------------------------------------------

### Requirements:

- [Vulkan](https://www.khronos.org/vulkan/)
- [FontConfig](https://www.freedesktop.org/wiki/Software/fontconfig/)
- [Freetype](https://www.freetype.org/)
- PkgConfig (currently used only to find harbfbuzz)
- [Harfbuzz](https://www.freedesktop.org/wiki/Software/HarfBuzz/)
- GLSLC: spirv compiler, included in [LunarG SDK](https://www.lunarg.com/vulkan-sdk/) (building only)
- [xxd](https://linux.die.net/man/1/xxd): generate headers with precompiled shaders (building only)
- [GLFW](http://www.glfw.org/) (only for running demo app)
- CMake

if glslc or xxd are not present, a precompiled version of the shaders is stored in the git tree.

### Building

```bash
git clone https://github.com/jpbruyere/vkvg.git     # Download source code from github
cd vkvg                                             # Enter the source directory
git submodule update --init --recursive             # Dowload vkhelpers sources
mkdir build					    # Create build directory
cd build
cmake ..					    # Run CMake, optionaly setup glslc path
make						    # Run Make
```

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


