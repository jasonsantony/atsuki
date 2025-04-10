cmake_minimum_required(VERSION 3.14)
project(atsuki)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# Use pkg-config to find installed packages
find_package(PkgConfig REQUIRED)

# Use pkg-config to find glfw and OpenCV
pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(OpenCV REQUIRED opencv4)

# Include paths
include_directories(
    ${GLFW_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    external/glad/include
    utils
)

link_directories(
    ${GLFW_LIBRARY_DIRS}
    ${OpenCV_LIBRARY_DIRS}
)

# Source files
set(SOURCES
    main.cpp
    render_pass.cpp
    shader_utils.cpp
    external/glad/src/gl.c
)

add_executable(atsuki ${SOURCES})

# Enable strict compiler warnings
target_compile_options(atsuki PRIVATE -Wall -Wextra -Werror)

# Link against libraries
target_link_libraries(atsuki
    ${GLFW_LIBRARIES}
    ${OpenCV_LIBRARIES}
    ${CMAKE_DL_LIBS}
)
