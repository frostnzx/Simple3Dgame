cmake_minimum_required(VERSION 3.0)
project(CyborgCollector3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path to LearnOpenGL
set(LOGL_DIR "${CMAKE_SOURCE_DIR}/../LearnOpenGL")

# Include directories
include_directories(${LOGL_DIR}/includes)
include_directories(${LOGL_DIR}/build/configuration)

# Find system packages
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW3 REQUIRED glfw3)

# Find Assimp
find_package(assimp QUIET)
if(NOT assimp_FOUND)
    pkg_check_modules(ASSIMP REQUIRED assimp)
    set(ASSIMP_LIBS ${ASSIMP_LIBRARIES})
    set(ASSIMP_INC ${ASSIMP_INCLUDE_DIRS})
else()
    set(ASSIMP_LIBS assimp::assimp)
    set(ASSIMP_INC "")
endif()

# Build GLAD
add_library(glad STATIC ${LOGL_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${LOGL_DIR}/includes)

# Build stb_image
add_library(stb_image STATIC ${LOGL_DIR}/src/stb_image.cpp)
target_include_directories(stb_image PUBLIC ${LOGL_DIR}/includes)

# Main executable
add_executable(game game.cpp)

target_include_directories(game PRIVATE
    ${GLFW3_INCLUDE_DIRS}
    ${ASSIMP_INC}
)

target_link_libraries(game
    glad
    stb_image
    ${GLFW3_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${ASSIMP_LIBS}
    X11 Xrandr Xinerama Xi Xxf86vm Xcursor
    GL dl pthread freetype
)

# Copy shaders to build directory
file(GLOB SHADERS "${CMAKE_SOURCE_DIR}/*.vs" "${CMAKE_SOURCE_DIR}/*.fs")
foreach(SHADER ${SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    configure_file(${SHADER} ${CMAKE_BINARY_DIR}/${SHADER_NAME} COPYONLY)
endforeach()
