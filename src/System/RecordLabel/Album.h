#pragma once

#include "Core/SatyrCore.h"
#include "System/RecordLabel/Song.h"
#include "System/RecordLabel/Producer.h"

typedef struct SyrAlbumConfig
{
    char name[32];
} SyrAlbumConfig;

typedef struct SyrAlbum SyrAlbum;

SyrResult SyrAlbum_Initialize(const char name[32],
    SyrAlbum** album);

void SyrAlbum_AddSongs(SyrAlbum* album, SyrSong** songs, const size_t count);

SyrResult SyrAlbum_RecordSongs(SyrAlbum* album,
    SyrProducer* producer,
    SyrTimelineTicket* timelineTicketRef);

SyrResult SyrAlbum_Release(SyrAlbum* album);
const char* SyrAlbum_GetName(const SyrAlbum* album);

void SyrAlbum_Destroy(SyrAlbum* album);
