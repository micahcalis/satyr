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

find_package(Vulkan REQUIRED)

# ==========================================
# VMA IMPLEMENTATION (VENDOR)
# ==========================================

set(VMA_IMP_FILE "${CMAKE_BINARY_DIR}/vma_implementation.cpp")
if(NOT EXISTS "${VMA_IMP_FILE}")
    file(WRITE "${VMA_IMP_FILE}"
        "#define VMA_IMPLEMENTATION\n"
        "#include \"VMA/vk_mem_alloc.h\"\n")
endif()

# ==========================================
# DR_LIBS IMPLEMENTATION (VENDOR)
# ==========================================

set(DR_LIBS_IMP_FILE "${CMAKE_BINARY_DIR}/dr_libs_implementation.c")
if(NOT EXISTS "${DR_LIBS_IMP_FILE}")
    file(WRITE "${DR_LIBS_IMP_FILE}"
        "#define DR_WAV_IMPLEMENTATION\n"
        "#include \"dr_wav.h\"\n\n"
        "#define DR_MP3_IMPLEMENTATION\n"
        "#include \"dr_mp3.h\"\n")
endif()

# ==========================================
# MINIAUDIO IMPLEMENTATION (VENDOR)
# ==========================================

set(MINI_AUDIO_IMP_FILE "${CMAKE_BINARY_DIR}/miniaudio_implementation.c")
if(NOT EXISTS "${MINI_AUDIO_IMP_FILE}")
    file(WRITE "${MINI_AUDIO_IMP_FILE}"
        "#define MINIAUDIO_IMPLEMENTATION\n"         
"#include \"miniaudio.h\"\n\n")
endif()

# ==========================================
# TARGET EXECUTABLE & INCLUDES
# ==========================================

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.c")

add_executable(Satyr ${SRC_FILES} ${VMA_IMP_FILE} ${DR_LIBS_IMP_FILE} ${MINI_AUDIO_IMP_FILE})

target_include_directories(Satyr PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/vendor/VMA
    ${CMAKE_SOURCE_DIR}/vendor/dr_libs
    ${CMAKE_SOURCE_DIR}/vendor/miniaudio
)

target_link_libraries(Satyr PRIVATE 
    Vulkan::Vulkan
)

# ==========================================
# AUDIO ASSETS PIPELINE
# ==========================================

set(AUDIO_INPUT_DIR "${CMAKE_SOURCE_DIR}/assets/audio")
set(AUDIO_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/audio")

file(GLOB_RECURSE AUDIO_FILES 
    CONFIGURE_DEPENDS 
    "${AUDIO_INPUT_DIR}/*.wav" 
    "${AUDIO_INPUT_DIR}/*.mp3"
)

set(AUDIO_COPY_OUTPUTS "")

foreach(AUDIO_FILE ${AUDIO_FILES})
    file(RELATIVE_PATH REL_AUDIO_PATH "${AUDIO_INPUT_DIR}" "${AUDIO_FILE}")
    set(DST_AUDIO_PATH "${AUDIO_OUTPUT_DIR}/${REL_AUDIO_PATH}")
    get_filename_component(DST_DIR "${DST_AUDIO_PATH}" DIRECTORY)

    add_custom_command(
        OUTPUT "${DST_AUDIO_PATH}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${AUDIO_FILE}" "${DST_AUDIO_PATH}"
        DEPENDS "${AUDIO_FILE}"
        COMMENT "Copying Audio Asset: ${REL_AUDIO_PATH}"
        VERBATIM
    )

    list(APPEND AUDIO_COPY_OUTPUTS "${DST_AUDIO_PATH}")
endforeach()

add_custom_target(AudioAssets DEPENDS ${AUDIO_COPY_OUTPUTS})
add_dependencies(Satyr AudioAssets)

# ==========================================
# SHADER COMPILATION PIPELINE (SLANG)
# ==========================================

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_program(SLANGC_EXECUTABLE slangc REQUIRED)

add_custom_target(Shaders) 

# 1. Define module directory and grab module files automatically
set(SLANG_MODULES_DIR "${CMAKE_SOURCE_DIR}/assets/shaders/modules")
file(GLOB_RECURSE SLANG_MODULE_FILES CONFIGURE_DEPENDS "${SLANG_MODULES_DIR}/*.slang")

function(add_slang_shader_target TARGET)
    cmake_parse_arguments("SHADER" "" "" "SOURCES" ${ARGN})
    list(GET SHADER_SOURCES 0 MAIN_SOURCE)
    
    file(RELATIVE_PATH REL_SHADER_PATH "${CMAKE_SOURCE_DIR}/assets/shaders" "${MAIN_SOURCE}")
    get_filename_component(REL_DIR "${REL_SHADER_PATH}" DIRECTORY)
    get_filename_component(SHADER_BASE_NAME "${MAIN_SOURCE}" NAME_WE)

    set(SHADERS_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/shaders/${REL_DIR}")
    
    set(OUTPUT_SPV "${SHADERS_OUTPUT_DIR}/${SHADER_BASE_NAME}.spv")

    add_custom_command(
        OUTPUT  ${OUTPUT_SPV} 
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADERS_OUTPUT_DIR}"
        
        # RUN SLANGC SECOND: Generate the Bytecode and include the modules folder path
        COMMAND ${SLANGC_EXECUTABLE} "${MAIN_SOURCE}"
                -target spirv -fvk-use-entrypoint-name -profile spirv_1_4 -emit-spirv-directly 
                -matrix-layout-column-major
                -I "${SLANG_MODULES_DIR}" 
                -o "${OUTPUT_SPV}"
                -O3
                
        WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/shaders"
        
        DEPENDS ${SHADER_SOURCES} ${COOKER_SCRIPT} ${SLANG_MODULE_FILES}
        COMMENT "Compiling Slang Shader: ${REL_SHADER_PATH}"
        VERBATIM
    )
    
    add_custom_target(${TARGET} DEPENDS ${OUTPUT_SPV})
endfunction()

# 2. GLOB ALL SHADERS RECURSIVELY WITH CONFIGURE_DEPENDS
file(GLOB_RECURSE ALL_RAW_SHADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/assets/shaders/*.slang")

# 3. FILTER OUT THE MODULES FOLDER
set(ALL_SHADERS "")
foreach(SHADER_FILE ${ALL_RAW_SHADERS})
    string(FIND "${SHADER_FILE}" "/modules/" IS_MODULE)
    string(FIND "${SHADER_FILE}" "\\modules\\" IS_MODULE_WIN)
    if(IS_MODULE EQUAL -1 AND IS_MODULE_WIN EQUAL -1)
        list(APPEND ALL_SHADERS "${SHADER_FILE}")
    endif()
endforeach()

# 4. CREATE TARGETS FOR EVERY COMPUTE SHADER FOUND
foreach(SHADER_FILE ${ALL_SHADERS})
    file(RELATIVE_PATH REL_SHADER_PATH "${CMAKE_SOURCE_DIR}/assets/shaders" "${SHADER_FILE}")
    string(REPLACE "/" "_" SAFE_TARGET_NAME "${REL_SHADER_PATH}")
    string(REPLACE "\\" "_" SAFE_TARGET_NAME "${SAFE_TARGET_NAME}")
    get_filename_component(SAFE_TARGET_NAME "${SAFE_TARGET_NAME}" NAME_WE)
    
    set(TARGET_NAME "Shader_${SAFE_TARGET_NAME}")
    add_slang_shader_target(${TARGET_NAME} SOURCES "${SHADER_FILE}")
    
    add_dependencies(Shaders ${TARGET_NAME})
endforeach()

# 5. Link the shader compilation workflow directly to your main executable
add_dependencies(Satyr Shaders)