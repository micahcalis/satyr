#include "Melody.h"

typedef struct SyrMelody
{
    SyrList(SyrChord*) chords;
    char name[32];
} SyrMelody;

SyrResult SyrMelody_Initialize(const char name[32], SyrMelody** melody)
{
    *melody = SYR_NEW(*melody);
    (*melody)->chords = NULL;
    SYR_STR_COPY((*melody)->name, name);

    return SYR_RESULT_SUCCESS;
}

void SyrMelody_AddChords(SyrMelody* melody, SyrChord** chords, size_t count)
{
    if (melody == NULL || chords == NULL || count == 0)
        return;

    uint32_t newCount = SyrList_Count(melody->chords) + count;
    if (newCount > SYR_MAX_CHORDS)
    {
        SYR_ERROR("Chord Count overflow: %d (max: %u)", newCount, SYR_MAX_CHORDS);
        return;
    }

    SyrList_PushRange(melody->chords, chords, count);
}

void SyrMelody_PrintChords(SyrMelody* melody)
{
    for (size_t i = 0; i < SyrList_Count(melody->chords); i++)
    {
        SYR_LOG("Chord: %s, in Melody: %s", SyrChord_GetName(melody->chords[i]), melody->name);
    }
}

SyrChord* SyrMelody_GetChord(const SyrMelody* melody, const uint32_t index)
{
    if (index > SyrList_Count(melody->chords))
    {
        SYR_ERROR("Chord Index out of Range in Melody: %s", melody->name);
        return NULL;
    }

    return melody->chords[index];
}

size_t SyrMelody_GetChordCount(const SyrMelody* melody)
{
    return SyrList_Count(melody->chords);
}

const char* SyrMelody_GetName(const SyrMelody* melody)
{
    return melody->name;
}

SyrMetronomeConfig* SyrMelody_GetMetronomeConfigBody(const SyrMelody* melody)
{
    if (melody == NULL)
        return NULL;

    SyrMetronomeConfig* metronomeConfig = SYR_NEW(metronomeConfig);
    metronomeConfig->phaseConfigs = NULL;

    size_t chordCount = SyrMelody_GetChordCount(melody);

    for (size_t i = 0; i < chordCount; i++)
    {
        SyrMetronomePhaseConfig phaseConfig = {
            .bindings = NULL,
            .requiresMaster = false};

        size_t instrumentCount = SyrChord_GetInstrumentCount(
            SyrMelody_GetChord(melody, (uint32_t)i));

        for (size_t j = 0; j < instrumentCount; j++)
        {
            SyrInstrumentBinding binding = {
                .instrumentIndex = 0,
                .instrumentSlot = SYR_INSTRUMENT_SLOT_0};

            SyrList_Push(phaseConfig.bindings, binding);
        }

        phaseConfig.bindingCount = (uint32_t)SyrList_Count(phaseConfig.bindings);
        SyrList_Push(metronomeConfig->phaseConfigs, phaseConfig);
    }

    metronomeConfig->phaseCount = (uint32_t)SyrList_Count(metronomeConfig->phaseConfigs);

    return metronomeConfig;
}

SyrResult SyrMelody_PlayChord(SyrMelody* melody,
    const uint32_t index,
    SyrCommandBuffer* commandBuffer)
{
    return SYR_RESULT_SUCCESS;
}

void SyrMelody_Destroy(SyrMelody* melody)
{
    if (melody == NULL)
        return;

    if (melody->chords != NULL)
    {
        for (size_t i = 0; i < SyrList_Count(melody->chords); i++)
        {
            SyrChord_Destroy(melody->chords[i]);
        }

        SyrList_Free(melody->chords);
    }

    free(melody);
}
