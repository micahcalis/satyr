#pragma once

#include "Core/SatyrCore.h"
#include "System/RecordLabel/Song.h"
#include "System/RecordLabel/Producer.h"
#include "System/RecordLabel/MasterDisc.h"

typedef struct SyrAlbumConfig
{
    char name[32];
} SyrAlbumConfig;

typedef struct SyrAlbum SyrAlbum;

SyrResult SyrAlbum_Initialize(const char name[32],
    SyrAlbum** album);

void SyrAlbum_AddSongs(SyrAlbum* album, SyrSong** songs, const size_t count);

void SyrAlbum_SetMasterDisc(SyrAlbum* album,
    SyrMasterDisc* masterDisc);

SyrResult SyrAlbum_RecordSongs(SyrAlbum* album,
    SyrProducer* producer,
    SyrTimelineTicket* timelineTicket);

SyrResult SyrAlbum_CreateDiscAsset(SyrAlbum* album,
    SyrDiscAsset** discAsset);

size_t SyrAlbum_GetAlbumTotalSize(const SyrAlbum* album);
uint32_t SyrAlbum_GetSongCount(const SyrAlbum* album);
const char* SyrAlbum_GetName(const SyrAlbum* album);

void SyrAlbum_Destroy(SyrAlbum* album);
