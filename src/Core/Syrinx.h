#pragma once

#include "SatyrCore.h"
#include "System/RecordLabel/Notes.h"
#include "System/RecordLabel/Chord.h"
#include "System/RecordLabel/Melody.h"
#include "System/RecordLabel/Producer.h"
#include "System/RecordLabel/Instrument.h"

typedef struct SyrSyrinx SyrSyrinx;

SyrSyrinx* SyrSyrinx_Create(const SyrConfig* config);
SyrResult SyrSyrinx_InitializeVulkan(SyrSyrinx* syrinx, const SyrConfig* config);

SyrNoteBuffer* SyrSyrinx_CreateNoteBuffer(SyrSyrinx* syrinx,
    const SyrNotesData notesData);

SyrChord* SyrSyrinx_CreateChord(SyrSyrinx* syrinx,
    const SyrChordConfig* config);

SyrMelody* SyrSyrinx_CreateMelody(SyrSyrinx* syrinx,
    const SyrMelodyConfig* config);

SyrProducer* SyrSyrinx_CreateProducer(SyrSyrinx* syrinx,
    const SyrProducerConfig* config);

SyrInstrument* SyrSyrinx_CreateInstrument(SyrSyrinx* syrinx,
    const SyrInstrumentConfig* config);

void SyrSyrinx_Destroy(SyrSyrinx* syrinx);
