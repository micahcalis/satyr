#include "Album.h"

typedef struct SyrAlbum
{
    SyrList(SyrSong*) songs;
    char name[32];
} SyrAlbum;

SyrResult SyrAlbum_Initialize(const char name[32],
    SyrAlbum** album)
{
    (*album) = SYR_NEW(*album);
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
    SyrTimelineTicket* timelineTicket)
{
    SyrCommandBuffer* commandBuffer = SyrProducer_GetCommandBuffer(producer);

    if (SyrCommandBuffer_Begin(commandBuffer) != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to Begin CommandBuffer for Album: %s", album->name);
        return SYR_RESULT_FAILED;
    }

    for (size_t i = 0; i < SyrList_Count(album->songs); i++)
    {
        if (SyrSong_Record(album->songs[i], commandBuffer) != SYR_RESULT_SUCCESS)
        {
            SYR_ERROR("Failed to Record Song (name: %s), for Album (name: %s)",
                SyrSong_GetName(album->songs[i]),
                album->name);

            SyrCommandBuffer_Reset(commandBuffer);
            return SYR_RESULT_FAILED;
        }
    }

    *timelineTicket = SyrProducer_NewReleaseTicket(producer, album->name);

    if (SyrCommandBuffer_EndSubmit(commandBuffer,
            SyrProducer_GetTimelineSemaphore(producer),
            timelineTicket)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to End & Submit CommandBuffer for Album: %s", album->name);
        SyrCommandBuffer_Reset(commandBuffer);
        timelineTicket->id = SYR_INVALID_TICKET_ID;
        return SYR_RESULT_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrAlbum_Release(SyrAlbum* album);

void SyrAlbum_Reset(SyrAlbum* album)
{
    SyrList_Clear(album->songs);
}

const char* SyrAlbum_GetName(const SyrAlbum* album)
{
    return album->name;
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
