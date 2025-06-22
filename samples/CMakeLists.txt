cmake_minimum_required(VERSION 3.14)
project(samples)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse
  GIT_TAG        3eda91b2e1ce7d569f84ba295507c4cd8fd96910 #v3.2
)

FetchContent_MakeAvailable(argparse)

set(SAMPLES_DATA_ROOT "${CMAKE_SOURCE_DIR}/tests/data" CACHE STRING "Data path for samples")

file(GLOB SAMPLES_SRC "*.cpp" "tests/*.cpp")

add_executable("${PROJECT_NAME}" ${SAMPLES_SRC})

target_compile_definitions("${PROJECT_NAME}" PUBLIC
    SAMPLES_DATA_ROOT="${GTEST_DATA_ROOT}"
)
target_include_directories("${PROJECT_NAME}" PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/include"
  ${GLFW3_INCLUDE_DIR}
  ${Vulkan_INCLUDE_DIRS}
  "${CMAKE_SOURCE_DIR}/vkh/include"
  "${CMAKE_SOURCE_DIR}/vkh/src"
)
target_link_libraries("${PROJECT_NAME}"
  vkvg
  ${Vulkan_LIBRARIES}
  ${GLFW3_LIBRARY}
  argparse
)





