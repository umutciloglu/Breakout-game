# Minimum version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name and language
project("FreeGLUT Template")

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

# Add executable
add_executable(FreeGLUT-App

 "Source/Main.cpp"
 "Source/glad.c"

)

# Set include directories.
target_include_directories(FreeGLUT-App PUBLIC 
${CMAKE_SOURCE_DIR}/Include
${CMAKE_SOURCE_DIR}/ThirdParty/freeglut-3.6.0/include
)

# Link against FreeGLUT (static).
add_subdirectory("ThirdParty/freeglut-3.6.0")
target_link_libraries(FreeGLUT-App PUBLIC freeglut_static)
