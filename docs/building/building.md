---
layout: default
title: Building
nav_order: 2
has_children: true
permalink: /docs/building
---

## Requirements:

- [CMake](https://cmake.org/): version > 12.
- [Vulkan](https://www.khronos.org/vulkan/)
- [FontConfig](https://www.freedesktop.org/wiki/Software/fontconfig/)
- [Freetype](https://www.freetype.org/)
- [Harfbuzz](https://www.freedesktop.org/wiki/Software/HarfBuzz/)
- GLSLC: spirv compiler, included in [LunarG SDK](https://www.lunarg.com/vulkan-sdk/) (building only)
- [xxd](https://linux.die.net/man/1/xxd): generate headers with precompiled shaders (building only)
- [GLFW](http://www.glfw.org/): optional, if present tests are built.

if `glslc` or `xxd` are not present, a precompiled version of the shaders is stored in the git tree.
