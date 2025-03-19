cmake_minimum_required(VERSION 3.14)
project(unit_tests)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        b514bdc898e2951020cbdca1304b75f5950d1f59 #v1.15.2
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

set(GTEST_PNG_ROOT "${CMAKE_SOURCE_DIR}/ReferenceImages" CACHE STRING "Reference png root path")

add_executable("${PROJECT_NAME}"
  main.cpp
  device.cpp
  surface.cpp
  context.cpp
  basicDraw.cpp
)
target_compile_definitions("${PROJECT_NAME}" PUBLIC
    GTEST_PNG_ROOT="${GTEST_PNG_ROOT}"
)
target_include_directories("${PROJECT_NAME}" PUBLIC
  "${CMAKE_SOURCE_DIR}/include"
)
target_link_libraries(
  "${PROJECT_NAME}"
  GTest::gtest_main
  vkvg
)

include(GoogleTest)
gtest_discover_tests("${PROJECT_NAME}")
