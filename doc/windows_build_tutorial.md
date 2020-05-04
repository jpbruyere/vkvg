<h1 align="center">
 Tutorial for Windows with Visual Studio IDE:
</h1>

0 - In order to follow all the steps from this tutorial you must have Visual Studio installed on your machine. The version which we used was the VS 2017, but other versions should work (i.e. VS19 and VS15). If you do not have the VS program installed the steps which are presented in the following section might still have some value for you. <br>

1 - Make sure that Git is installed on your machine by typing on the command line<br>
```bash
> git --version
```

If the output of such command is: <br>
```bash
git version 2.19.1.windows.1
```

Then you have git installed on your machine. If an error is given, then install git on your machine.<br> 

2 - To install all the necessary libraries we first need to install the vcpkg package manager (the process can be done manually but it is easier this way) which allows one to link projects with a large array of libraries already developed in C\C++. The instalation of vcpkg is trivial, you just open a command line, and write <br>
```bash
> git clone https://github.com/Microsoft/vcpkg.git
> cd vcpkg
> .\bootstrap-vcpkg.bat
```
3 - Now we have to install the dependencies for the vkvg library which are :<br> 

- CMake -> Used to define projects across several operating systems (OS)<br>
- Vulkan -> The API to interact with the GPU to create images which you are already probabily familiar<br>
- FontConfig -> The library used to find the configuration of the fonts from the OS<br>
- Freetype -> Library used to render the fonts<br>
- Harfbuzz -> Library used to shape the text .<br>
- GLFW -> Library used to create a window independent from the underlying OS<br>

To install all these libraries we can simply write (assuming you are still on vcpkg directory):<br>
```bash
> vcpkg search vulkan
```

To find if the library already has a port for the vcpkg packadge manager<br>

4 - So now we know that the library is currently usable from the vcpkg program so we can now write:<br>
```bash
> vcpkg install vulkan
```

And do the same drill for all the other libraries...<br>
```bash
> vcpkg install freetype
> vcpkg install fontconfig
>...
```

5 - What we just did was that we download the source files from the open sourced projects and we compiled them with the MSVN compiler so we know we won't be facing any incompatibilities when compiling our tutorial. Now we can download the source code for the vkvg library by typing:<br>
```bash
> cd ..
```
On the command line to get out of the vcpkg directory and then write:<br>
```bash
> git clone https://github.com/jpbruyere/vkvg.git
```
To download the code to a local repository<br>

6 - Now we can use the IDE from CMake to generate the .sln file which can be processed by visual studio. Open the IDE and select the directory which contains the source code (it should be something like <i>'C:\...\vkvg'</i>). Now select the folder where you want to generate the project to. In our case we created a folder called bin (<i>'C:\...\vkvg\bin'</i>) and selected that file.<br>

7 - Now configure the project. This action generated a window where you can select the target build you want, in our case it is <i>Visual Studio 15 2017 Win64</i> and we must specify the toolchain file for cross-compiling (because it is the vcpkg program which knowns where all the libraries we previously downloaded currently are). It should be a file located in <i>'C:\...\vcpkg\scripts\buildsystems\vcpkg.cmake'</i> which contains information to be used by cmake.<br>

8 - Now we can finish the project and if everything went well we can now select the options of the project for this specific build. On the bottom of the GUI there are some options related with the VKVG libraries. You will unselect the <strong>VKVG_LCD_FONT_FILTER</strong> because we did not compile the FreeType library with this option enabled. Now press generate project and then procede to press open project. This should open the Visual Studio IDE.<br>

9 - Now you can see the the possible built configurations.To build the project (the <strong>ALL BUILD</strong> should be the Start Up project) press F7 or press build (top of the screen) and then build solution. If everything goes well we now have a .lib and .dll folder to use in our projects.<br>

10 - The project is compiled and if you go to the <i>'C:\...\vkvg\bin\CMakeFiles\Debug'</i> directory you will find all the .dlls and .libs files to integrate in your projects.<br>
