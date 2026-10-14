cmake_minimum_required(VERSION 3.14)
project(msdf)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  msdfgen
  GIT_REPOSITORY https://github.com/Chlumsky/msdfgen.git
  GIT_TAG        1874bcf7d9624ccc85b4bc9a85d78116f690f35b #v1.13
)

FetchContent_MakeAvailable(msdfgen)
