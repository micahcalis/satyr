#pragma once

// Core
#include "Core/SatyrCore.h"
#include "Core/Application.h"
#include "Core/Syrinx.h"

// Vulkan
#include "Core/Vulkan/Allocations.h"
#include "Core/Vulkan/Allocator.h"
#include "Core/Vulkan/Descriptor.h"
#include "Core/Vulkan/Device.h"
#include "Core/Vulkan/Pipeline.h"
#include "Core/Vulkan/ShaderModule.h"
#include "Core/Vulkan/VulkInstance.h"

#ifdef SATYR_IMPLEMENTATION

    // Core
    #include "Core/Application.c"
    #include "Core/Syrinx.c"

    // Vulkan
    #include "Core/Vulkan/Allocations.c"
    #include "Core/Vulkan/Allocator.c"
    #include "Core/Vulkan/Descriptor.c"
    #include "Core/Vulkan/Device.c"
    #include "Core/Vulkan/Pipeline.c"
    #include "Core/Vulkan/ShaderModule.c"
    #include "Core/Vulkan/VulkInstance.c"

#endif // SATYR_IMPLEMENTATION
