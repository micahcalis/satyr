#pragma once

#include "Core/SatyrCore.h"
#include "System/RecordPlayer/Vinyl.h"

#define SYR_MAX_VINYLS 512
#define SYR_INVALID_VINYL_ID 0

typedef struct SyrRecordPlayer SyrRecordPlayer;

SyrResult SyrRecordPlayer_Initialize(SyrRecordPlayer** recordPlayer);

SyrVinylId SyrRecordPlayer_CreateVinyl(SyrRecordPlayer* recordPlayer,
    SyrVinylConfig* config);

SyrVinyl* SyrRecordPlayer_GetVinyl(SyrRecordPlayer* recordPlayer,
    SyrVinylId id);

SyrResult SyrRecordPlayer_DestroyVinyl(SyrRecordPlayer* recordPlayer,
    SyrVinylId id);

void SyrRecordPlayer_Destroy(SyrRecordPlayer* recordPlayer);
