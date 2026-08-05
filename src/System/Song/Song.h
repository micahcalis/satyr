#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/CommandBuffer.h"

typedef struct SyrSong SyrSong;

SyrResult SyrSong_Initialize(SyrSong** song);
void SyrSong_Record(SyrSong* song, SyrCommandBuffer* commandBuffer);
void SyrSong_Destroy(SyrSong* song);
