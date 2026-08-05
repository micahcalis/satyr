#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/CommandBuffer.h"
#include "System/RecordLabel/Song.h"
#include "System/RecordLabel/Producer.h"

typedef struct SyrAlbum SyrAlbum;

SyrResult SyrAlbum_Initialize(SyrCommandBuffer* commandBuffer,
    SyrAlbum** album);

void SyrAlbum_BeginRecording(SyrAlbum* album, const char name[32]);
void SyrAlbum_RecordSong(SyrAlbum* album, SyrSong* song);

void SyrAlbum_Release(SyrAlbum* album, SyrProducer* producer);
void SyrAlbum_Reset(SyrAlbum* album);

void SyrAlbum_Destroy(SyrAlbum* album);
