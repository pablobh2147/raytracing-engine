cmake_minimum_required(VERSION 3.10)

# Generate compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Hoshizora)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler-specific optimization flags
add_compile_options(-march=native -O3 -ffast-math -funroll-loops)

# Find Vulkan SDK
find_package(Vulkan REQUIRED)

# Third-party libraries
add_subdirectory(lib/vma)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/glm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/stb)

# Source files
file(GLOB_RECURSE SOURCES src/*.cpp)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator)

# Shader compilation
find_program(GLSLC glslc HINTS Vulkan::glslc)

file(GLOB_RECURSE SHADER_SOURCES shaders/*.comp)

set(SHADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SHADER_OUTPUT ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv)
    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${GLSLC} ${SHADER} -o ${SHADER_OUTPUT} -I${CMAKE_CURRENT_SOURCE_DIR}/shaders
        DEPENDS ${SHADER}
        COMMENT "Compiling shader: ${SHADER_NAME}"
    )
    list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${SHADER_OUTPUTS})
add_dependencies(${PROJECT_NAME} Shaders)
