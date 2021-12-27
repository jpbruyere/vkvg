#  Build on Windows with Visual Studio IDE:

(*Tested with VS 2017*)
Ensure [CMake and c/c++ support options](https://docs.microsoft.com/en-us/visualstudio/install/modify-visual-studio?view=vs-2019) are installed along with Visual studio.

The easyest way to proceed is to open a visual studio command prompt, and issue commands manually. Once dependencies are build and vkvg project files have been created, you may use the ide.

## Build dependencies
Make sure that [Git](https://git-scm.com/download/win) is installed on your machine by typing on the command line (cmd):
```bash
> git --version
git version 2.19.1.windows.1
```

The only required dependency is vulkan, the header and library. To compile shaders you'll also need the `glslc` compiler. The easyest way is to install the the latest [vulkan sdk](https://vulkan.lunarg.com/) from lunarg, and set the environment variable `VULKAN_SDK` to the installation path of the sdk.

The following 3 libraries help with text shaping and font handling, but are optional.
- Fontconfig: find font file from a short name with style.
- Freetype: glyph rendering library, may be replaced by stb_truetype.
- Harfbuzz: text shaping library, help handling complex glyph combination.

The last dependency is [GLFW](https://www.glfw.org/), required by on screen samples to create and handle the native windows. If not present, an offscreen test will be optionaly built.

All those dependencies may be installed on windows with [vcpkg package manager](https://docs.microsoft.com/en-us/cpp/build/vcpkg?view=vs-2019).

```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```
Then install libraries, use the triplet corresponding with your architecture:
```bash
vcpkg install fontconfig:x64-windows freetype:x64-windows harfbuzz:x64-windows glfw3:x64-windows
```

Add the path to the shared libraries binaries to your **PATH** environment variable, they are in %vcpkg-root%/installed/x64-windows/bin/.

To make those libraries available for vsstudio use the [system wide integration](https://vcpkg.readthedocs.io/en/latest/examples/installing-and-using-packages/#vsmsbuild-project-user-wide-integration) from vcpkg.
```bash
vcpkg integrate install
```
## CMake configuration
Exit vcpkg directory and clone the vkvg repository and its submodule vkh:
```bash
git clone --recursive https://github.com/jpbruyere/vkvg.git
cd vkvg
#create a build directory
mkdir build
```
To generate the solution file (.sln) from the build directory we use cmake with the toolchain file provided by vcpkg and we also disable the lcd font filtering (FreeType is built without [lcd fonts](https://en.wikipedia.org/wiki/Subpixel_rendering) by default).
```
cmake .. -DCMAKE_TOOLCHAIN_FILE=..\..\vcpkg\scripts\buildsystems\vcpkg.cmake -DVKVG_LCD_FONT_FILTER=off
```
If you compile vkvg without dependencies, you may run cmake without toolchain: `cmake ..`
To adjust later compilation options, you may recall cmake command, or use the cmake-gui command.
```bash
cmake-gui ..
```
Now you should have the vkvg.sln and all the projects file into your build directory. You may build the complete solution on the command line with (from a visual studio command prompt):
```bash
msbuild vkvg.sln
```
