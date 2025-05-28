cmake_minimum_required(VERSION 3.2)
project(hw-gl)

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(GLEW REQUIRED glew)
pkg_check_modules(GLM REQUIRED glm)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ⚠️ 隱藏特定 warning（可改為 -w 完全關閉）
add_definitions(-DGL_SILENCE_DEPRECATION)


if(APPLE)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(COREVIDEO_FRAMEWORK CoreVideo)
    set(EXTRA_MAC_LIBS
        ${COCOA_FRAMEWORK}
        ${IOKIT_FRAMEWORK}
        ${COREVIDEO_FRAMEWORK}
    )
endif()

add_library(commonLib INTERFACE)
target_include_directories(commonLib INTERFACE
    ${GLFW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
)
target_link_directories(commonLib INTERFACE
    ${GLFW_LIBRARY_DIRS}
    ${GLEW_LIBRARY_DIRS}
    ${GLM_LIBRARY_DIRS}
)
target_link_libraries(commonLib INTERFACE
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLM_LIBRARIES}
    ${EXTRA_MAC_LIBS}
)

add_executable(main main.cpp cone_builder.cpp)
target_link_libraries(main PRIVATE commonLib)

