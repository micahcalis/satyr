#include "Album.h"

typedef struct SyrAlbum
{
    SyrCommandBuffer* commandBuffer;
    SyrList(SyrSong*) songs;
    char name[32];
} SyrAlbum;

SyrResult SyrAlbum_Initialize(SyrCommandBuffer* commandBuffer,
    const char name[32],
    SyrAlbum** album)
{
    (*album) = SYR_NEW(*album);
    (*album)->commandBuffer = commandBuffer;
    (*album)->songs = NULL;
    SYR_STR_COPY((*album)->name, name);

    return SYR_RESULT_SUCCESS;
}

void SyrAlbum_AddSongs(SyrAlbum* album, SyrSong** songs, const size_t count)
{
    if (album == NULL || songs == NULL || count == 0)
        return;

    SyrList_PushRange(album->songs, songs, count);
}

SyrResult SyrAlbum_RecordSongs(SyrAlbum* album,
    SyrProducer* producer,
    const SyrTimelineTicket** timelineTicket)
{
    if (SyrCommandBuffer_Begin(album->commandBuffer) != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to Begin CommandBuffer for Album: %s", album->name);
        return SYR_RESULT_FAILED;
    }

    for (size_t i = 0; i < SyrList_Count(album->songs); i++)
    {
        if (SyrSong_Record(album->songs[i], album->commandBuffer) != SYR_RESULT_SUCCESS)
        {
            SYR_ERROR("Failed to Record Song (name: %s), for Album (name: %s)",
                SyrSong_GetName(album->songs[i]),
                album->name);

            SyrCommandBuffer_Reset(album->commandBuffer);
            return SYR_RESULT_FAILED;
        }
    }

    *timelineTicket = SyrProducer_NewReleaseTicket(producer, album->name);

    if (SyrCommandBuffer_EndSubmit(album->commandBuffer,
            SyrProducer_GetTimelineSemaphore(producer),
            *timelineTicket)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to End & Submit CommandBuffer for Album: %s", album->name);
        return SYR_RESULT_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrAlbum_Release(SyrAlbum* album);

void SyrAlbum_Reset(SyrAlbum* album)
{
    SyrList_Clear(album->songs);
}

void SyrAlbum_Destroy(SyrAlbum* album)
{
    if (album == NULL)
        return;

    for (size_t i = 0; i < SyrList_Count(album->songs); i++)
    {
        SyrSong_Destroy(album->songs[i]);
    }

    SyrList_Free(album->songs);
    free(album);
}
