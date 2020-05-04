
```bash
git clone https://github.com/jpbruyere/vkvg.git     # Download source code from github
cd vkvg                                             # Enter the source directory
git submodule update --init --recursive             # Dowload vkhelpers sources
mkdir build					    # Create build directory
cd build
cmake ..					    # Run CMake, optionaly setup glslc path
make						    # Run Make
```
