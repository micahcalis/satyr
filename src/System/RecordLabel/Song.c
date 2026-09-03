#include "Song.h"

typedef struct SyrSong
{
    SyrAudioBuffer* masterBuffer;
    SyrList(SyrInstrument*) instruments;
    SyrMelody* melody;
    SyrMetronome* metronome;
    char name[32];
} SyrSong;

SyrResult SyrSong_Initialize(SyrAudioBuffer* masterBuffer,
    SyrMelody* melody,
    SyrMetronome* metronome,
    const char name[32],
    SyrSong** song)
{
    (*song) = SYR_NEW(*song);
    (*song)->masterBuffer = masterBuffer;
    (*song)->instruments = NULL;
    (*song)->melody = melody;
    (*song)->metronome = metronome;
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

static SyrResult SyrSong_PrePlayChord(SyrSong* song,
    SyrChord* chord,
    SyrCommandBuffer* commandBuffer,
    const size_t phaseIndex)
{
    if (SyrMetronome_BindPhaseBuffers(song->metronome,
            commandBuffer,
            chord,
            phaseIndex,
            song->masterBuffer,
            song->instruments)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to bind Phase Buffers for Song: %s", song->name);
        return SYR_RESULT_FAILED;
    }

    if (SyrMetronome_RecordPhaseBarriers(song->metronome,
            commandBuffer,
            phaseIndex,
            song->masterBuffer,
            song->instruments)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to record Phase Barriers for Song: %s", song->name);
        return SYR_RESULT_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

static SyrResult SyrSong_BindReleaseTransfer(SyrSong* song)
{
    return SYR_RESULT_SUCCESS;
}

SyrResult SyrSong_Record(SyrSong* song,
    SyrCommandBuffer* commandBuffer)
{
    for (size_t i = 0; i < SyrMelody_GetChordCount(song->melody); i++)
    {
        SyrChord* chord = SyrMelody_GetChord(song->melody, (uint32_t)i);

        if (SyrSong_PrePlayChord(song, chord, commandBuffer, i) != SYR_RESULT_SUCCESS)
        {
            return SYR_RESULT_FAILED;
        }

        if (SyrMelody_PlayChord(song->melody,
                commandBuffer,
                (uint32_t)i,
                SyrMetronome_GetDispatchSamples(song->metronome, i))
            != SYR_RESULT_SUCCESS)
        {
            SYR_ERROR("Failed to Play Chord for Song (name: %s) with Melody (name: %s), Chord (name: %s)!",
                song->name,
                SyrMelody_GetName(song->melody),
                SyrChord_GetName(chord));

            return SYR_RESULT_FAILED;
        }
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrSong_Release(SyrSong* song,
    SyrCommandBuffer* commandBuffer,
    SyrBufferAllocation* releaseBuffer,
    const size_t offset)
{
    return SyrCommandBuffer_CopyBuffer(commandBuffer,
        song->masterBuffer->timeAllocation,
        releaseBuffer,
        SyrSong_GetMasterSize(song),
        offset);
}

size_t SyrSong_GetMasterSize(const SyrSong* song)
{
    return song->masterBuffer->totalSamples * sizeof(SyrTimeAudioSample);
}

const char* SyrSong_GetName(const SyrSong* song)
{
    return song->name;
}

uint64_t SyrSong_GetTotalFrames(const SyrSong* song)
{
    return song->masterBuffer->totalSamples / (uint64_t)song->masterBuffer->sampleMode;
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

    SYR_FREE(song);
}

