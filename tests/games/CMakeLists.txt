cmake_minimum_required(VERSION 3.14)

project(games C CXX)

FUNCTION (buildgame GAME_NAME)
    SET_TARGET_PROPERTIES(${GAME_NAME} PROPERTIES
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
    )
    TARGET_INCLUDE_DIRECTORIES(${GAME_NAME}
        PRIVATE
            ${Vulkan_INCLUDE_DIRS}
            ${GLFW3_INCLUDE_DIR}
            "${CMAKE_CURRENT_SOURCE_DIR}/../include"
            "${CMAKE_CURRENT_SOURCE_DIR}/common"
            "${CMAKE_CURRENT_SOURCE_DIR}/../vkh/include"
    )
    TARGET_LINK_LIBRARIES(${GAME_NAME} PRIVATE
            vkvg
            vkh
            ${Vulkan_LIBRARIES}
            ${GLFW3_LIBRARY}
    )
ENDFUNCTION (buildgame)

ADD_LIBRARY(game_common OBJECT common/VkvgApp.cpp common/VkvgApp.hpp)

TARGET_INCLUDE_DIRECTORIES(game_common
    PRIVATE
        ${Vulkan_INCLUDE_DIRS}
        ${GLFW3_INCLUDE_DIR}
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"
        "${CMAKE_CURRENT_SOURCE_DIR}/common"
        "${CMAKE_CURRENT_SOURCE_DIR}/../vkh/include"
)

file(GLOB GAMES "*.cpp")

FOREACH(GAME ${GAMES})
    GET_FILENAME_COMPONENT(GAME_NAME ${GAME} NAME_WE)
    ADD_EXECUTABLE(${GAME_NAME} ${GAME} $<TARGET_OBJECTS:game_common>)
    buildgame(${GAME_NAME})
ENDFOREACH()