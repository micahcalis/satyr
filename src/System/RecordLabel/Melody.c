#include "Melody.h"

typedef struct SyrMelody
{
    SyrList(SyrChord*) chords;
} SyrMelody;

SyrResult SyrMelody_Initialize(SyrMelody** melody)
{
    *melody = SYR_NEW(*melody);
    (*melody)->chords = NULL;

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
