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

    if (count + SyrList_Count(album->songs) > SYR_MAX_DISC_SONGS)
    {
        SYR_ERROR("Failed to add Songs to Album (%s), Song overflow! (max is 64)", album->name);
        return;
    }

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

static SyrResult SyrAlbum_InitializeDiscAudioAssets(SyrDiscAsset* discAsset,
    SyrAlbum* album)
{
    for (uint32_t i = 0; i < discAsset->discCount; i++)
    {
        discAsset->audioAssets[i] = SYR_NEW(discAsset->audioAssets[i]);
        SyrAudioAsset* audioAsset = discAsset->audioAssets[i];
        SyrSong* song = album->songs[i];

        audioAsset->channels = (uint8_t)SYR_AUDIO_ASSET_SAMPLE_MODE_MONO;
        audioAsset->sampleRate = SYR_AUDIO_SAMPLE_RATE;
        SYR_STR_COPY(audioAsset->name, SyrSong_GetName(song));
        audioAsset->totalFrames = SyrSong_GetTotalFrames(song);
        audioAsset->pcmData = SYR_ALLOC_ARRAY(float, audioAsset->totalFrames);
    }

    return SYR_RESULT_SUCCESS;
}

static inline void SyrAlbum_GetSongSizes(SyrAlbum* album, size_t* sizes)
{
    size_t songCount = SyrList_Count(album->songs);

    for (size_t i = 0; i < songCount; i++)
    {
        sizes[i] = SyrSong_GetMasterSize(album->songs[i]);
    }
}

SyrResult SyrAlbum_CreateDiscAsset(SyrAlbum* album,
    SyrDiscAsset** discAsset)
{
    if (album->masterDisc == NULL)
    {
        SYR_ERROR("Can't Create Disc Asset with NULL Master Disc (Album: %s)!", album->name);
        return SYR_RESULT_FAILED;
    }

    uint32_t songCount = (uint32_t)SyrList_Count(album->songs);
    if (songCount == 0)
    {
        SYR_ERROR("Cannot create Disc Asset: Album (name: %s) has 0 songs!", album->name);
        return SYR_RESULT_FAILED;
    }

    if (songCount > SYR_MAX_DISC_SONGS)
    {
        SYR_ERROR("Album (name: %s) exceeds maximum (%u) allowed songs per disc!", album->name, SYR_MAX_DISC_SONGS);
        return SYR_RESULT_FAILED;
    }

    *discAsset = SYR_NEW(*discAsset);
    (*discAsset)->discCount = songCount;
    memset((*discAsset)->audioAssets, 0, sizeof((*discAsset)->audioAssets));
    SYR_STR_COPY((*discAsset)->albumName, album->name);

    if (SyrAlbum_InitializeDiscAudioAssets(*discAsset, album) != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to Initialize Disc Audio Assets for Album: %s", album->name);
        SyrDiscAsset_Destroy(*discAsset, true);
        *discAsset = NULL;
        return SYR_RESULT_FAILED;
    }

    size_t songSizes[songCount];
    SyrAlbum_GetSongSizes(album, songSizes);

    if (SyrMasterDisc_BurnAsset(album->masterDisc,
            *discAsset,
            songSizes,
            songCount,
            SyrAlbum_GetAlbumTotalSize(album))
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to Burn Disc Asset for Album: %s", album->name);
        SyrDiscAsset_Destroy(*discAsset, true);
        *discAsset = NULL;
        return SYR_RESULT_FAILED;
    }

    return SYR_RESULT_SUCCESS;
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
