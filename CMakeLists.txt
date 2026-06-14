cmake_minimum_required(VERSION 3.16)
project(homeward LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# All module sources live under src/ (m1..m5 + common + main.cpp).
file(GLOB_RECURSE HOMEWARD_SRC CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(homeward ${HOMEWARD_SRC})
target_include_directories(homeward PRIVATE "${CMAKE_SOURCE_DIR}/src")

# --- OpenGL ---------------------------------------------------------------
find_package(OpenGL REQUIRED)
target_link_libraries(homeward PRIVATE ${OPENGL_LIBRARIES})

# --- FreeGLUT (MSYS2 UCRT64: mingw-w64-ucrt-x86_64-freeglut) ---------------
# Prefer CMake's finder; fall back to the MinGW import library by name.
find_package(GLUT QUIET)
if(TARGET GLUT::GLUT)
    target_link_libraries(homeward PRIVATE GLUT::GLUT)
else()
    target_link_libraries(homeward PRIVATE freeglut)
endif()

# GLU (gluOrtho2D / gluPerspective / gluLookAt / gluSphere).
target_link_libraries(homeward PRIVATE glu32)

# --- GLM (optional — installed but the code hand-builds its matrices) ------
find_package(glm CONFIG QUIET)
if(glm_FOUND)
    target_compile_definitions(homeward PRIVATE HAVE_GLM)
    target_link_libraries(homeward PRIVATE glm::glm)
    message(STATUS "GLM found (optional).")
endif()

# FreeGLUT on Windows wants this define when linking the static-ish import lib.
target_compile_definitions(homeward PRIVATE FREEGLUT_STATIC=0)
