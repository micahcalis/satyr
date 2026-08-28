#pragma once

#include "Core/SatyrCore.h"
#include "System/RecordPlayer/Vinyl.h"
#include "System/RecordPlayer/Voice.h"

typedef struct SyrRecordPlayer SyrRecordPlayer;

SyrResult SyrRecordPlayer_Initialize(const SyrConfig* config,
    SyrRecordPlayer** recordPlayer);

SyrVinylId SyrRecordPlayer_CreateVinyl(SyrRecordPlayer* recordPlayer,
    const SyrVinylConfig* config);

SyrVinyl* SyrRecordPlayer_GetVinyl(SyrRecordPlayer* recordPlayer,
    const SyrVinylId id);

SyrResult SyrRecordPlayer_DestroyVinyl(SyrRecordPlayer* recordPlayer,
    const SyrVinylId id);

SyrVoiceId SyrRecordPlayer_PlayVinyl(SyrRecordPlayer* recordPlayer,
    const SyrVinylId vinylId,
    const float volume,
    const float pitch);

SyrVoice* SyrRecordPlayer_GetVoice(SyrRecordPlayer* recordPlayer,
    const SyrVoiceId voiceId);

SyrResult SyrRecordPlayer_StopVoice(SyrRecordPlayer* recordPlayer,
    const SyrVoiceId voiceId);

void SyrRecordPlayer_Destroy(SyrRecordPlayer* recordPlayer);
