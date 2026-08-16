#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/TimelineSemaphore.h"
#include "Core/Vulkan/CommandPool.h"

typedef enum
{
    SYR_PRODUCER_PRIORITY_HIGH = 0,
    SYR_PRODUCER_PRIORITY_MEDIUM = 1,
    SYR_PRODUCER_PRIORITY_LOW = 2,
} SyrProducerPriority;

typedef struct SyrProducerConfig
{
    const char name[32];
    SyrProducerPriority priority;
} SyrProducerConfig;

typedef struct SyrProducer SyrProducer;

SyrResult SyrProducer_Initialize(SyrCommandPool* commandPool,
    SyrTimelineSemaphore* timelineSemaphore,
    SyrProducerPriority priority,
    const char name[32],
    SyrProducer** producer);

void SyrProducer_Update(SyrProducer* producer);
SyrCommandBuffer* SyrProducer_GetCommandBuffer(SyrProducer* producer);
const SyrTimelineTicket SyrProducer_NewReleaseTicket(SyrProducer* producer, const char albumName[32]);
SyrTimelineSemaphore* SyrProducer_GetTimelineSemaphore(SyrProducer* producer);
bool SyrProducer_IsTicketComplete(SyrProducer* producer, const SyrTimelineTicket* ticket);
const char* SyrProducer_GetName(const SyrProducer* producer);

void SyrProducer_Destroy(SyrProducer* producer);

