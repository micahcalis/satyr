cmake_minimum_required(VERSION 3.15)

# Enable both C and CXX languages so CMake can build the VMA C++ implementation
project(Satyr C CXX) 

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)

# Satyr core code standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# VMA requires at least C++14 to compile
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

find_package(Vulkan REQUIRED)

FetchContent_Declare(
    VMA
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(VMA)

set(VMA_IMP_FILE "${CMAKE_BINARY_DIR}/vma_implementation.cpp")
if(NOT EXISTS "${VMA_IMP_FILE}")
    file(WRITE "${VMA_IMP_FILE}"
        "#define VMA_IMPLEMENTATION\n"
        "#include \"vk_mem_alloc.h\"\n")
endif()

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.c")

add_executable(Satyr ${SRC_FILES} ${VMA_IMP_FILE})

target_include_directories(Satyr PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(Satyr PRIVATE 
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
)