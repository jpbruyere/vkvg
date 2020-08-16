---
sort: 1
---

# Install Dependencies

`glslc` and `xxd` are required to compile the shaders. If one of them is not installed, a precompiled version of the shaders is stored in the git tree.

### build tools
```bash
sudo apt install git cmake gcc g++ xxd
```

### vulkan lib and tools

###### from debian packages
```bash
sudo apt install libvulkan-dev mesa-vulkan-drivers
```
###### from lunarg sdk
```bash

```
 
##### build dependencies
```bash
sudo apt-get install libfontconfig-dev libfreetype-dev libharfbuzz-dev  
```


## Getting the sources from GitHub

```bash
#fetch sources from github
git clone --recursive https://github.com/jpbruyere/vkvg.git    
cd vkvg
# Create build directory
mkdir build
cd build
# Run CMake, optionaly setup glslc path
cmake ..
make
```
