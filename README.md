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
  <a href="https://www.paypal.me/GrandTetraSoftware">
    <img src="https://img.shields.io/badge/Donate-PayPal-green.svg">
  </a>
</p>
</h1>

### What is vkvg?

**vkvg** is a multiplatform **c** library for drawing 2D vector graphics with [Vulkan](https://www.khronos.org/vulkan/).

[Cairo](https://www.cairographics.org/) is missing a Vulkan backend, so I decided to start one myself trying to keep my api as close to Cairo as possible. Maybe vkvg could serve as a starting point for Cairo maintainers to start their Vulkan backend.

### Current status: 

vkvg is in early development stage, and no guarantee is given on the possible roadmap:

- Basic shape fill and stroke functional, using ears clipping.
- Basic painting operation.
- Font system with caching operational.
- Context should be thread safe, tests required.
- Nice logo.

### Roadmap

- Offscreen pattern building. 

