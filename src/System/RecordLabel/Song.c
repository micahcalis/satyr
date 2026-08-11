#include "Song.h"

typedef struct SyrSong
{
    SyrAudioBuffer* masterBuffer;
    SyrList(SyrInstrument*) instruments;
    SyrMelody* melody;
    char name[32];
} SyrSong;

SyrResult SyrSong_Initialize(SyrAudioBuffer* masterBuffer,
    SyrMelody* melody,
    const char name[32],
    SyrSong** song)
{
    (*song) = SYR_NEW(*song);
    (*song)->masterBuffer = masterBuffer;
    (*song)->instruments = NULL;
    (*song)->melody = melody;
    SYR_STR_COPY((*song)->name, name);

    return SYR_RESULT_SUCCESS;
}

void SyrSong_AddInstruments(SyrSong* song, SyrInstrument** instruments, size_t count)
{
    if (song == NULL || instruments == NULL || count == 0)
        return;

    uint32_t newCount = SyrList_Count(song->instruments) + count;
    if (newCount > SYR_MAX_INSTRUMENTS)
    {
        SYR_ERROR("Instrument Count overflow: %d (max: %u)", newCount, SYR_MAX_INSTRUMENTS);
        return;
    }

    SyrList_PushRange(song->instruments, instruments, count);
}

void SyrSong_Record(SyrSong* song, SyrCommandBuffer* commandBuffer)
{
    // melody stuff that has to be figured out
}

void SyrSong_Destroy(SyrSong* song)
{
    if (song == NULL)
        return;

    if (song->masterBuffer != NULL)
    {
        SyrAudioBuffer_Destroy(song->masterBuffer);
    }

    if (song->instruments != NULL)
    {
        for (size_t i = 0; i < SyrList_Count(song->instruments); i++)
        {
            SyrInstrument_Destroy(song->instruments[i]);
        }

        SyrList_Free(song->instruments);
    }

    free(song);
}

