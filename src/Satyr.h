#pragma once

// Core
#include "Core/SatyrCore.h"
#include "Core/Syrinx.h"

// Core - Vulkan
#include "Core/Vulkan/Allocations.h"
#include "Core/Vulkan/Allocator.h"
#include "Core/Vulkan/Barrier.h"
#include "Core/Vulkan/CommandBuffer.h"
#include "Core/Vulkan/CommandPool.h"
#include "Core/Vulkan/Descriptor.h"
#include "Core/Vulkan/Device.h"
#include "Core/Vulkan/Pipeline.h"
#include "Core/Vulkan/ShaderModule.h"
#include "Core/Vulkan/TimelineSemaphore.h"
#include "Core/Vulkan/VulkInstance.h"

// System - RecordLabel
#include "System/RecordLabel/Album.h"
#include "System/RecordLabel/Chord.h"
#include "System/RecordLabel/Instrument.h"
#include "System/RecordLabel/Melody.h"
#include "System/RecordLabel/Metronome.h"
#include "System/RecordLabel/Notes.h"
#include "System/RecordLabel/Producer.h"
#include "System/RecordLabel/RecordLabel.h"
#include "System/RecordLabel/Song.h"

// System - RecordPlayer
#include "System/RecordPlayer/AudioAsset.h"
#include "System/RecordPlayer/AudioDevice.h"
#include "System/RecordPlayer/RecordPlayer.h"
#include "System/RecordPlayer/Vinyl.h"
#include "System/RecordPlayer/Voice.h"

// Utilities
#include "Utilities/DynamicArray.h"
#include "Utilities/Math.h"
#include "Utilities/SatyrDebug.h"
#include "Utilities/SlotMap.h"

#ifdef SATYR_IMPLEMENTATION

    // Core
    #include "Core/Syrinx.c"

    // Core - Vulkan
    #include "Core/Vulkan/Allocations.c"
    #include "Core/Vulkan/Allocator.c"
    #include "Core/Vulkan/Barrier.c"
    #include "Core/Vulkan/CommandBuffer.c"
    #include "Core/Vulkan/CommandPool.c"
    #include "Core/Vulkan/Descriptor.c"
    #include "Core/Vulkan/Device.c"
    #include "Core/Vulkan/Pipeline.c"
    #include "Core/Vulkan/ShaderModule.c"
    #include "Core/Vulkan/TimelineSemaphore.c"
    #include "Core/Vulkan/VulkInstance.c"

    // System - RecordLabel
    #include "System/RecordLabel/Album.c"
    #include "System/RecordLabel/Chord.c"
    #include "System/RecordLabel/Instrument.c"
    #include "System/RecordLabel/Melody.c"
    #include "System/RecordLabel/Metronome.c"
    #include "System/RecordLabel/Notes.c"
    #include "System/RecordLabel/Producer.c"
    #include "System/RecordLabel/RecordLabel.c"
    #include "System/RecordLabel/Song.c"

    // System - RecordPlayer
    #include "System/RecordPlayer/AudioAsset.c"
    #include "System/RecordPlayer/AudioDevice.c"
    #include "System/RecordPlayer/RecordPlayer.c"
    #include "System/RecordPlayer/Vinyl.c"
    #include "System/RecordPlayer/Voice.c"

#endif // SATYR_IMPLEMENTATION
