#pragma once

#include "Core/SatyrCore.h"
#include "Core/Syrinx.h"

#define SYR_MAX_POLL_EVENTS 128

typedef enum
{
    SYR_PRODUCTION_STATE_UNRECORDED,
    SYR_PRODUCTION_STATE_RECORDED,
    SYR_PRODUCTION_STATE_RELEASED
} SyrProductionState;

typedef struct SyrProduction
{
    SyrProducer* producer;
    SyrAlbum* album;
    SyrTimelineTicket ticket;
} SyrProduction;

typedef struct SyrProductionEvent
{
    const SyrProduction* production;
    SyrProductionState state;
} SyrProductionEvent;

typedef struct SyrPollEvents
{
    SyrProductionEvent events[SYR_MAX_POLL_EVENTS];
    uint32_t count;
} SyrPollEvents;

typedef struct SyrRecordLabel SyrRecordLabel;

SyrResult SyrRecordLabel_Initialize(SyrSyrinx* syrinx,
    SyrRecordLabel** recordLabel);

SyrAlbum* SyrRecordLabel_NewAlbum(SyrRecordLabel* recordLabel,
    const SyrAlbumConfig* config);

SyrProducer* SyrRecordLabel_NewProducer(SyrRecordLabel* recordLabel,
    const SyrProducerConfig* config);

SyrResult SyrRecordLabel_StartProduction(SyrRecordLabel* recordLabel,
    SyrAlbum* album,
    SyrProducer* producer);

SyrResult SyrRecordLabel_PollEvents(SyrRecordLabel* recordLabel,
    SyrPollEvents* pollEvents);

void SyrRecordLabel_Destroy(SyrRecordLabel* recordLabel);
