#  Build on Windows with Visual Studio IDE:

(*Tested with VS 2017*)
Ensure [CMake and c/c++ support options](https://docs.microsoft.com/en-us/visualstudio/install/modify-visual-studio?view=vs-2019) are installed along with Visual studio.

The easyest way to proceed is to open a visual studio command prompt, and issue commands manually. Once dependencies are build and vkvg project files have been created, you may use the ide.

#### Build dependencies
Install the latest [vulkan sdk](https://vulkan.lunarg.com/) from lunarg.

Make sure that [Git](https://git-scm.com/download/win) is installed on your machine by typing on the command line (cmd):
```bash
> git --version
git version 2.19.1.windows.1
```
Install [vcpkg package manager](https://docs.microsoft.com/en-us/cpp/build/vcpkg?view=vs-2019) to build required libraries:
```bash 
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```
Then install the libraries required by vkvg, use the triplet corresponding with your architecture:
```bash 
vcpkg install fontconfig:x64-window freetype:x64-window harfbuzz:x64-window
```
Optionaly, install [GLFW] to build the samples:
```bash 
vcpkg install glfw:x64-window
```
Add the path to the shared libraries binaries to your **PATH** environment variable, they are in %vcpkg-root%/installed/x64-windows/bin/.

To make those libraries available for vsstudio use the [system wide integration](https://vcpkg.readthedocs.io/en/latest/examples/installing-and-using-packages/#vsmsbuild-project-user-wide-integration) from vcpkg.
```bash 
vcpkg integrate install
```
#### CMake configuration
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
To adjust later compilation options, you may recall cmake command, or use the cmake-gui command.
```bash
cmake-gui ..
```
Now you should have the vkvg.sln and all the projects file into your build directory. You may build the complete solution on the command line with:
```bash
msbuild vkvg.sln
```
