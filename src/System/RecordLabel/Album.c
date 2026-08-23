#include "Album.h"

typedef struct SyrAlbum
{
    SyrList(SyrSong*) songs;
    SyrMasterDisc* masterDisc;
    char name[32];
} SyrAlbum;

SyrResult SyrAlbum_Initialize(const char name[32],
    SyrAlbum** album)
{
    (*album) = SYR_NEW(*album);
    (*album)->songs = NULL;
    (*album)->masterDisc = NULL;
    SYR_STR_COPY((*album)->name, name);

    return SYR_RESULT_SUCCESS;
}

void SyrAlbum_AddSongs(SyrAlbum* album, SyrSong** songs, const size_t count)
{
    if (album == NULL || songs == NULL || count == 0)
        return;

    SyrList_PushRange(album->songs, songs, count);
}

void SyrAlbum_SetMasterDisc(SyrAlbum* album,
    SyrMasterDisc* masterDisc)
{
    if (album->masterDisc != NULL)
    {
        SyrMasterDisc_Destroy(album->masterDisc);
    }

    album->masterDisc = masterDisc;
}

static SyrResult SyrAlbum_ReleaseSongs(SyrAlbum* album,
    SyrCommandBuffer* commandBuffer)
{
    size_t offset = 0;

    for (size_t i = 0; i < SyrList_Count(album->songs); i++)
    {
        if (SyrSong_Release(album->songs[i],
                commandBuffer,
                SyrMasterDisc_GetDiscBufferAlloc(album->masterDisc),
                offset)
            != SYR_RESULT_SUCCESS)
        {
            SYR_ERROR("Failed to Release Song (name: %s), for Album (name: %s)",
                SyrSong_GetName(album->songs[i]),
                album->name);

            return SYR_RESULT_FAILED;
        }

        offset += SyrSong_GetMasterSize(album->songs[i]);
    }

    return SYR_RESULT_SUCCESS;
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

    if (album->masterDisc != NULL)
    {
        if (SyrAlbum_ReleaseSongs(album, commandBuffer) != SYR_RESULT_SUCCESS)
        {
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

void SyrAlbum_Reset(SyrAlbum* album)
{
    SyrList_Clear(album->songs);
}

size_t SyrAlbum_GetAlbumTotalSize(const SyrAlbum* album)
{
    size_t totalSize = 0;

    for (size_t i = 0; i < SyrList_Count(album->songs); i++)
    {
        totalSize += SyrSong_GetMasterSize(album->songs[i]);
    }

    return totalSize;
}

uint32_t SyrAlbum_GetSongCount(const SyrAlbum* album)
{
    return (uint32_t)SyrList_Count(album->songs);
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

    if (album->masterDisc != NULL)
    {
        SyrMasterDisc_Destroy(album->masterDisc);
    }

    SYR_FREE(album);
}
