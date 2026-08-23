#include "RecordLabel.h"

typedef struct SyrRecordLabel
{
    SyrSyrinx* syrinx;
    SyrList(SyrAlbum*) albums;
    SyrList(SyrProducer*) producers;
    SyrList(SyrProduction) activeProductions;
    uint64_t productionIdCounter;
} SyrRecordLabel;

SyrResult SyrRecordLabel_Initialize(SyrSyrinx* syrinx,
    SyrRecordLabel** recordLabel)
{
    if (syrinx == NULL)
    {
        SYR_ERROR("Can't construct Record Label with invalid Syrinx!");
        return SYR_RESULT_FAILED;
    }

    *recordLabel = SYR_NEW(*recordLabel);
    (*recordLabel)->syrinx = syrinx;
    (*recordLabel)->albums = NULL;
    (*recordLabel)->producers = NULL;
    (*recordLabel)->activeProductions = NULL;
    (*recordLabel)->productionIdCounter = 0;

    return SYR_RESULT_SUCCESS;
}

SyrAlbum* SyrRecordLabel_NewAlbum(SyrRecordLabel* recordLabel,
    const SyrAlbumConfig* config)
{
    SyrAlbum* newAlbum = SyrSyrinx_CreateAlbum(recordLabel->syrinx, config);

    if (newAlbum == NULL)
        return NULL;

    SyrList_Push(recordLabel->albums, newAlbum);

    return newAlbum;
}

SyrProducer* SyrRecordLabel_NewProducer(SyrRecordLabel* recordLabel,
    const SyrProducerConfig* config)
{
    SyrProducer* newProducer = SyrSyrinx_CreateProducer(recordLabel->syrinx, config);

    if (newProducer == NULL)
        return NULL;

    SyrList_Push(recordLabel->producers, newProducer);

    return newProducer;
}

static inline bool SyrRecordLabel_ContainsAlbum(const SyrRecordLabel* recordLabel,
    const SyrAlbum* album)
{
    for (size_t i = 0; i < SyrList_Count(recordLabel->albums); i++)
    {
        if (album == recordLabel->albums[i])
            return true;
    }

    return false;
}

static inline bool SyrRecordLabel_ContainsProducer(const SyrRecordLabel* recordLabel,
    const SyrProducer* producer)
{
    for (size_t i = 0; i < SyrList_Count(recordLabel->producers); i++)
    {
        if (producer == recordLabel->producers[i])
            return true;
    }

    return false;
}

static inline bool SyrRecordLabel_IsAlbumInProduction(const SyrRecordLabel* recordLabel,
    const SyrAlbum* album)
{
    for (size_t i = 0; i < SyrList_Count(recordLabel->activeProductions); i++)
    {
        if (album == recordLabel->activeProductions[i].album)
            return true;
    }

    return false;
}

static uint64_t SyrRecordLabel_NewProductionId(SyrRecordLabel* recordLabel)
{
    recordLabel->productionIdCounter++;
    return recordLabel->productionIdCounter;
}

SyrResult SyrRecordLabel_StartProduction(SyrRecordLabel* recordLabel,
    SyrAlbum* album,
    SyrProducer* producer,
    const SyrProductionType type,
    uint64_t* productionIdRef)
{
    if (SyrList_Count(recordLabel->activeProductions) >= SYR_MAX_POLL_EVENTS)
    {
        SYR_ERROR("Can't start Production, Current Productions count maximum Reached (128)");
        return SYR_RESULT_FAILED;
    }

    if (!SyrRecordLabel_ContainsAlbum(recordLabel, album)
        || !SyrRecordLabel_ContainsProducer(recordLabel, producer))
    {
        SYR_ERROR("Can't start Production, RecordLabel doesn't contain Album (name: %s) or Producer (name %s)!",
            SyrAlbum_GetName(album),
            SyrProducer_GetName(producer));

        return SYR_RESULT_FAILED;
    }

    if (SyrRecordLabel_IsAlbumInProduction(recordLabel, album))
    {
        SYR_ERROR("Can't start Production, RecordLabel is already producing Album (name: %s)!",
            SyrAlbum_GetName(album));

        return SYR_RESULT_FAILED;
    }

    SyrProduction production = {0};
    production.album = album;
    production.producer = producer;
    production.type = type;

    if (type == SYR_PRODUCTION_TYPE_RECORD_RELEASE)
    {
        SyrMasterDisc* masterDisc = SyrSyrinx_CreateMasterDisc(recordLabel->syrinx,
            album);

        if (masterDisc == NULL)
        {
            return SYR_RESULT_FAILED;
        }

        SyrAlbum_SetMasterDisc(album, masterDisc);
    }

    if (SyrAlbum_RecordSongs(album,
            producer,
            &production.ticket)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Can't start Production, Album (naem: %s) Record Songs Failure!",
            SyrAlbum_GetName(album));

        return SYR_RESULT_FAILED;
    }

    production.productionId = SyrRecordLabel_NewProductionId(recordLabel);

    if (productionIdRef != NULL)
    {
        *productionIdRef = production.productionId;
    }

    SyrList_Push(recordLabel->activeProductions, production);

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrRecordLabel_PollEvents(SyrRecordLabel* recordLabel,
    SyrPollEvents* pollEvents)
{
    if (recordLabel == NULL || pollEvents == NULL)
        return SYR_RESULT_FAILED;

    for (size_t i = 0; i < SyrList_Count(recordLabel->producers); i++)
    {
        SyrProducer_Update(recordLabel->producers[i]);
    }

    pollEvents->count = SyrList_Count(recordLabel->activeProductions);

    for (size_t i = 0; i < SyrList_Count(recordLabel->activeProductions); i++)
    {
        SyrProductionEvent* event = &pollEvents->events[i];
        SyrProduction* production = &recordLabel->activeProductions[i];
        SyrProductionState completionEvent = production->type == SYR_PRODUCTION_TYPE_RECORD
            ? SYR_PRODUCTION_STATE_RECORDED
            : SYR_PRODUCTION_STATE_RELEASED;

        event->state = SyrProducer_IsTicketComplete(production->producer, &production->ticket)
            ? completionEvent
            : SYR_PRODUCTION_STATE_UNRECORDED;

        event->production = *production;
    }

    return SYR_RESULT_SUCCESS;
}

void SyrRecordLabel_Destroy(SyrRecordLabel* recordLabel)
{
    if (recordLabel == NULL)
        return;

    SyrList_Free(recordLabel->activeProductions);

    for (size_t i = 0; i < SyrList_Count(recordLabel->albums); i++)
    {
        SyrAlbum_Destroy(recordLabel->albums[i]);
    }

    SyrList_Free(recordLabel->albums);

    for (size_t i = 0; i < SyrList_Count(recordLabel->producers); i++)
    {
        SyrProducer_Destroy(recordLabel->producers[i]);
    }

    SyrList_Free(recordLabel->producers);

    SYR_FREE(recordLabel);
}
