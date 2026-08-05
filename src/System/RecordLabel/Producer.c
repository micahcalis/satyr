#include "Producer.h"

typedef struct SyrProducer
{
    SyrTimelineSemaphore* timelineSemaphore;
    SyrTimelineTicket timelineTicket;
    char name[32];
} SyrProducer;

SyrResult SyrProducer_Initialize(SyrTimelineSemaphore* timelineSemaphore,
    const char name[32],
    SyrProducer** producer)
{
    (*producer) = SYR_NEW(*producer);
    (*producer)->timelineSemaphore = timelineSemaphore;
    (*producer)->timelineTicket.id = SYR_INVALID_TICKET_ID;
    (*producer)->timelineTicket.name[0] = '\0';
    strncpy_s((*producer)->name, sizeof((*producer)->name), name, sizeof((*producer)->name) - 1);

    return SYR_RESULT_SUCCESS;
}

void SyrProducer_Update(SyrProducer* producer)
{
    SyrTimelineSemaphore_UpdateSemaphoreCounter(producer->timelineSemaphore);
}

const SyrTimelineTicket* SyrProducer_NewReleaseTicket(SyrProducer* producer, const char albumName[32])
{
    char releaseName[64];
    snprintf(releaseName, sizeof(releaseName), "%s - %s", albumName, producer->name);
    producer->timelineTicket = SyrTimelineSemaphore_AssignTicket(producer->timelineSemaphore, releaseName);

    return &producer->timelineTicket;
}

SyrTimelineSemaphore* SyrProducer_GetTimelineSemaphore(SyrProducer* producer)
{
    return producer->timelineSemaphore;
}

void SyrProducer_Destroy(SyrProducer* producer)
{
    if (producer == NULL)
        return;

    if (producer->timelineSemaphore != NULL)
    {
        SyrTimelineSemaphore_Destroy(producer->timelineSemaphore);
    }

    free(producer);
}
