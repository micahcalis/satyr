#include "Producer.h"

typedef struct SyrProducer
{
    SyrCommandPool* commandPool;
    SyrCommandBuffer* commandBuffer;
    SyrTimelineSemaphore* timelineSemaphore;
    SyrProducerPriority priority;
    char name[32];
} SyrProducer;

SyrResult SyrProducer_Initialize(SyrCommandPool* commandPool,
    SyrTimelineSemaphore* timelineSemaphore,
    SyrProducerPriority priority,
    const char name[32],
    SyrProducer** producer)
{
    (*producer) = SYR_NEW(*producer);
    (*producer)->commandPool = commandPool;
    (*producer)->timelineSemaphore = timelineSemaphore;
    (*producer)->priority = priority;
    SYR_STR_COPY((*producer)->name, name);

    (*producer)->commandBuffer = SyrCommandPool_AllocateCommandBuffer(commandPool);

    if ((*producer)->commandBuffer == NULL)
    {
        SyrProducer_Destroy(*producer);
        *producer = NULL;
        return SYR_RESULT_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

void SyrProducer_Update(SyrProducer* producer)
{
    SyrTimelineSemaphore_UpdateSemaphoreCounter(producer->timelineSemaphore);
}

const SyrTimelineTicket SyrProducer_NewReleaseTicket(SyrProducer* producer, const char albumName[32])
{
    char releaseName[70];
    snprintf(releaseName, sizeof(releaseName), "%s - %s", albumName, producer->name);
    SyrTimelineTicket timelineTicket = SyrTimelineSemaphore_AssignTicket(producer->timelineSemaphore, releaseName);

    return timelineTicket;
}

SyrCommandBuffer* SyrProducer_GetCommandBuffer(SyrProducer* producer)
{
    return producer->commandBuffer;
}

SyrTimelineSemaphore* SyrProducer_GetTimelineSemaphore(SyrProducer* producer)
{
    return producer->timelineSemaphore;
}

bool SyrProducer_IsTicketComplete(SyrProducer* producer, const SyrTimelineTicket* ticket)
{
    return SyrTimelineSemaphore_IsTicketComplete(producer->timelineSemaphore, ticket);
}

const char* SyrProducer_GetName(const SyrProducer* producer)
{
    return producer->name;
}

void SyrProducer_Destroy(SyrProducer* producer)
{
    if (producer == NULL)
        return;

    if (producer->timelineSemaphore != NULL)
    {
        SyrTimelineSemaphore_Destroy(producer->timelineSemaphore);
    }

    if (producer->commandBuffer != NULL)
    {
        SyrCommandBuffer_Destroy(producer->commandBuffer);
    }

    if (producer->commandPool != NULL)
    {
        SyrCommandPool_Destroy(producer->commandPool);
    }

    SYR_FREE(producer);
}
