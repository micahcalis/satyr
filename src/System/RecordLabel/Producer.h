#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/TimelineSemaphore.h"

typedef struct SyrProducerConfig
{
    const char name[32];
} SyrProducerConfig;

typedef struct SyrProducer SyrProducer;

SyrResult SyrProducer_Initialize(SyrTimelineSemaphore* timelineSemaphore,
    const char name[32],
    SyrProducer** producer);

void SyrProducer_Update(SyrProducer* producer);
const SyrTimelineTicket* SyrProducer_NewReleaseTicket(SyrProducer* producer, const char albumName[32]);
SyrTimelineSemaphore* SyrProducer_GetTimelineSemaphore(SyrProducer* producer);

void SyrProducer_Destroy(SyrProducer* producer);

