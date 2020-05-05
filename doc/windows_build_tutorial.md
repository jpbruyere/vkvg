<h1 align="center">
 Tutorial for Windows with Visual Studio IDE:
</h1>

0 - In order to follow all the steps from this tutorial Visual Studio must be installed on your machine. The version used in this build tutorial was the VS 2017, but other versions should work (i.e. VS19 and VS15). If VS is not installed the following steps might still have some value for your specific build. <br>

1 - Make sure that Git is installed on your machine by typing on the command line (cmd)<br>
```bash
> git --version
```

If the output of such command is: <br>
```bash
git version 2.19.1.windows.1
```

Then git is installed on your machine. If an error is given, then [install git on your machine](https://git-scm.com/download/win).<br> 

2 - To install all the dependencies one can use a packadge manager to simplify the required steps. The [vcpkg package manager](https://docs.microsoft.com/en-us/cpp/build/vcpkg?view=vs-2019) allows one to link projects with a large array of libraries already developed in C\C++. The instalation of **vcpkg** is trivial. First open a command line and write: <br>
```bash
> git clone https://github.com/Microsoft/vcpkg.git
> cd vcpkg
> .\bootstrap-vcpkg.bat
```
3 - The dependencies required by **vkvg** are :<br> 

- [CMake](https://cmake.org/) -> Used to define projects across several operating systems (OS)<br>
- [Vulkan](https://www.khronos.org/vulkan/) -> The API to interact with the GPU to create images which you are already probabily familiar<br>
- [FontConfig](https://www.freedesktop.org/wiki/Software/fontconfig/) -> The library used to find the configuration of the fonts from the OS<br>
- [Freetype](https://www.freetype.org/) -> Library used to render the fonts<br>
- [Harfbuzz](https://www.freedesktop.org/wiki/Software/HarfBuzz/) -> Library used to shape the text .<br>
- [GLFW](http://www.glfw.org/) -> Library used to create a window independent from the underlying OS<br>

To install all these libraries write on the cmd (assuming you are still on **vcpkg** directory):<br>
```bash
> vcpkg search vulkan
```
To find if the library already has a port for **vcpkg**<br>

4 - If the port exists write on the cmd:<br>
```bash
> vcpkg install vulkan
```

And do the same drill for all the other libraries...<br>
```bash
> vcpkg install freetype
> vcpkg install fontconfig
>...
```

With the previous commands the source files from the open sourced project is now on the machine and they have been compiled with MSVN so that no incompatibilities emerge further down the line.

5 - To download the **vkvg** library type:<br>
```bash
> cd ..
```
On the command line to get out of the **vcpkg** directory and then write:<br>
```bash
> git clone https://github.com/jpbruyere/vkvg.git
> cd vkvg 
> git submodule update --init --recursive 
> mkdir build
```
To download the code to a local repository, download the vkh dependency and create the directory to build.<br>

6 - To generate the .sln file which can be processed by visual studio go to the CMake IDE and lauch it. Select the directory which contains the source code (it should be something like <i>'C:\...\vkvg'</i>). Now select the folder where you want to generate the project to. For this test build select <i>'C:\...\vkvg\build'</i> as the destination of the binaries.<br>

7 - Now configure the project. This action generated a window where you can select the target build you want, if you are using VS17 it should be something like <i>Visual Studio 15 2017 Win64</i>, and the toolchain file for cross-compiling must be specified (because vcpkg generates a file which simplifies the use of the libraries previously downloaded). It should be a file located in <i>'C:\...\vcpkg\scripts\buildsystems\vcpkg.cmake'</i> which contains information used by cmake.<br>

8 - If everything went well it is time to select the options of the project for this specific build. On the bottom of the GUI there are some options related with the **vkvg** library. Unselect the <strong>VKVG_LCD_FONT_FILTER</strong> because **vcpkg** does not build FreeType with this option enabled. Now generate the sln project and procede to open the project with Visual Studio.<br>

9 - To build the project (the <strong>ALL BUILD</strong> project should be the Start Up project) press F7 or press build (top of the screen) and then press build solution. If everything goes well you finally have the .lib and .dll files to use in our projects. These files can be found in the <i>'C:\...\vkvg\build\CMakeFiles\Debug'</i> directory (this directory is dependent of the folders created on step 6) <br>
