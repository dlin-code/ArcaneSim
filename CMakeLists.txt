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

# Compile shaders automatically
set(SHADER_DIR		${CMAKE_SOURCE_DIR}/shaders)
set(SHADER_OUT_DIR	${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY	${SHADER_OUT_DIR})

if(NOT DEFINED Vulkan_GLSLC_EXECUTABLE)
  set(Vulkan_GLSLC_EXECUTABLE "$ENV{VULKAN_SDK}/Bin/glslc.exe")
endif()

set(GLSL_SHADERS
  ${SHADER_DIR}/shader.vert
  ${SHADER_DIR}/shader.frag
  ${SHADER_DIR}/skybox.vert
  ${SHADER_DIR}/skybox.frag
  ${SHADER_DIR}/instance_shader.vert
  ${SHADER_DIR}/particle.vert
  ${SHADER_DIR}/particle.frag
  ${SHADER_DIR}/particle.comp
)

set(SPV_SHADERS "")
foreach(GLSL ${GLSL_SHADERS})
  get_filename_component(FNAME ${GLSL} NAME_WE)
  get_filename_component(EXT ${GLSL} EXT)

  if(EXT STREQUAL ".vert")
    set(SPV ${SHADER_OUT_DIR}/${FNAME}_vert.spv)
  elseif(EXT STREQUAL ".frag")
    set(SPV ${SHADER_OUT_DIR}/${FNAME}_frag.spv)
  elseif(EXT STREQUAL ".comp")
    set(SPV ${SHADER_OUT_DIR}/${FNAME}_comp.spv)
  else()
    message(FATAL_ERROR "Unknown shader type: ${GLSL}")
  endif()

  add_custom_command(
    OUTPUT ${SPV}
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -o ${SPV}
    DEPENDS ${GLSL}
    COMMENT "Compiling ${GLSL} -> ${SPV}"
    VERBATIM
  )
  list(APPEND SPV_SHADERS ${SPV})
endforeach()

add_custom_target(ShadersTarget DEPENDS ${SPV_SHADERS})

# Create executable
add_executable(ArcaneSim ${SRC_FILES} "src/VulkanApp.cpp" "src/VulkanApp.h" "src/camera.cpp" "src/camera.h"  "src/particle.h")

add_dependencies(ArcaneSim ShadersTarget)

# Link libraries
target_link_libraries(ArcaneSim Vulkan::Vulkan glfw)