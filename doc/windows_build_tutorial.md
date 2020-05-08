#  Build on Windows with Visual Studio IDE:

(*Tested with VS 2017*)

#### Build dependencies
Make sure that [Git](https://git-scm.com/download/win) is installed on your machine by typing on the command line (cmd):
```bash
> git --version
git version 2.19.1.windows.1
```
Install [vcpkg package manager](https://docs.microsoft.com/en-us/cpp/build/vcpkg?view=vs-2019) to build all required build dependencies:
```bash 
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```
Then install the libraries required by vkvg:
```bash 
vcpkg install fontconfig freetype harfbuzz vulkan
```
Optionaly, install [GLFW] to build the samples:
```bash 
vcpkg install glfw
```
#### CMake configuration
Exit vcpkg directory and clone the vkvg repository:
```bash
git clone --recursive https://github.com/jpbruyere/vkvg.git
cd vkvg
#create a build directory
mkdir build
```
To generate the solution file (.sln), use CMake IDE. Select the vkvg root folder as the source directory and the build folder we've just create in it.

Configure the project and select the target build. (*for VS17 it should be something like `Visual Studio 15 2017 Win64`*).

Also specify the **toolchain file** for cross-compiling provided by **vcpkg** (*C:\...\vcpkg\scripts\buildsystems\vcpkg.cmake*).

On the bottom of the GUI there are some options related with the **vkvg** library. Unselect the **VKVG_LCD_FONT_FILTER** (FreeType is built without [lcd fonts](https://en.wikipedia.org/wiki/Subpixel_rendering) by default).

Now generate the sln project and procede to open the project with Visual Studio.<br>

#### Build vkvg
To build the project (the **ALL BUILD** project should be the Start Up project) press **F7** or press build (top of the screen) and then press build solution. If everything goes well you finally have the .lib and .dll files to use in our projects. These files can be found in the *C:\...\vkvg\build\CMakeFiles\Debug* directory.
