#pragma once

#include "Core/SatyrCore.h"

#define SYR_INSTRUMENT_SSBO_COUNT 1

typedef struct SyrInstrument SyrInstrument;

void SyrInstrument_Destroy(SyrInstrument* instrument);
