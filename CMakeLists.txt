cmake_minimum_required(VERSION 3.15)

# 1. Fixed project definition: Tell CMake the project name is Satyr and the language is C
project(Satyr C) 

# Generate compile_commands.json for clangd language server
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 2. Tell CMake to put the compiled executable directly in the ./bin/ folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)

# Force C11 standard for modern features (like designated initializers and atomics)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find the Vulkan library on your system
find_package(Vulkan REQUIRED)

# Define the executable and the C files to compile
add_executable(Satyr src/main.c)

# Link Vulkan to your executable
target_link_libraries(Satyr PRIVATE Vulkan::Vulkan)