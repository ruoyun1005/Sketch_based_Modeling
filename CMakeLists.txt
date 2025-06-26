cmake_minimum_required(VERSION 3.10)
project(SketchModeling)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-DGL_SILENCE_DEPRECATION)  # macOS 下隱藏 OpenGL deprecation 警告

# 找你要的 library
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

# 把 external/imgui 當作子專案編譯
#add_subdirectory(external/imgui)

# 你自己的程式碼
set(SRCS
    main.cpp
    cylinder_builder.cpp
    csg.cpp
)

# 建 executable
add_executable(main ${SRCS})

# include 路徑
target_include_directories(main PRIVATE
    #${CMAKE_SOURCE_DIR}/external/imgui    # ImGui headers
    ${GLEW_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
)

# link libraries
target_link_libraries(main PRIVATE
    #imgui
    GLEW::GLEW
    glfw
    ${OPENGL_LIBRARIES}
)
