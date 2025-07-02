cmake_minimum_required(VERSION 3.10)

# Project name and language
project(ArcaneSim VERSION 0.1 LANGUAGES CXX)

# Set C++ version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Vulkan (from Vulkan SDK)
find_package(Vulkan REQUIRED)

# GLFW (local subdirectory)
add_subdirectory(external/glfw)
include_directories(external/glfw/include)

# GLM is header-only
include_directories(external/glm)  # Optional for math later

# Source files
file(GLOB_RECURSE SRC_FILES src/*.cpp)

# Create executable
add_executable(ArcaneSim ${SRC_FILES} "src/VulkanApp.cpp" "src/VulkanApp.h")

# Link libraries
target_link_libraries(ArcaneSim Vulkan::Vulkan glfw)