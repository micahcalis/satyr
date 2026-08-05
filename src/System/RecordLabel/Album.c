#include "Album.h"

typedef struct SyrAlbum
{
    SyrCommandBuffer* commandBuffer;
    SyrList(SyrSong*) recordedSongs;
    char name[32];
} SyrAlbum;

SyrResult SyrAlbum_Initialize(SyrCommandBuffer* commandBuffer,
    SyrAlbum** album)
{
    (*album) = SYR_NEW(*album);
    (*album)->commandBuffer = commandBuffer;
    (*album)->recordedSongs = NULL;
    (*album)->name[0] = '\0';

    return SYR_RESULT_SUCCESS;
}

void SyrAlbum_BeginRecording(SyrAlbum* album, const char name[32])
{
    strncpy_s(album->name, sizeof(album->name), name, sizeof(album->name) - 1);
    SyrCommandBuffer_Begin(album->commandBuffer);
}

void SyrAlbum_RecordSong(SyrAlbum* album, SyrSong* song)
{
    SyrSong_Record(song, album->commandBuffer);
    SyrList_Push(album->recordedSongs, song);
}

void SyrAlbum_Release(SyrAlbum* album, SyrProducer* producer)
{
    const SyrTimelineTicket* ticket = SyrProducer_NewReleaseTicket(producer, album->name);

    SyrCommandBuffer_EndSubmit(album->commandBuffer,
        SyrProducer_GetTimelineSemaphore(producer),
        ticket);

    SyrAlbum_Reset(album);
}

void SyrAlbum_Reset(SyrAlbum* album)
{
    SyrList_Clear(album->recordedSongs);
}

void SyrAlbum_Destroy(SyrAlbum* album)
{
    if (album == NULL)
        return;

    SyrList_Free(album->recordedSongs);
    SyrCommandBuffer_Destroy(album->commandBuffer);

    free(album);
}
