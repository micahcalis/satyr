#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Device.h"

#define SYR_INVALID_TICKET_ID UINT64_MAX

typedef struct SyrTimelineSemaphore SyrTimelineSemaphore;

typedef struct SyrTimelineTicket
{
    char name[64];
    uint64_t id;
} SyrTimelineTicket;

SyrResult SyrTimelineSemaphore_Initialize(SyrDevice* device,
    SyrTimelineSemaphore** timelineSemaphore);

SyrTimelineTicket SyrTimelineSemaphore_AssignTicket(SyrTimelineSemaphore* timelineSemaphore, char name[64]);
void SyrTimelineSemaphore_UpdateSemaphoreCounter(SyrTimelineSemaphore* timelineSemaphore);
bool SyrTimelineSemaphore_IsTicketComplete(SyrTimelineSemaphore* timelineSemaphore, const SyrTimelineTicket* ticket);
VkSemaphore SyrTimelineSemaphore_GetSemaphoreHandle(SyrTimelineSemaphore* timelineSemaphore);
VkTimelineSemaphoreSubmitInfo SyrTimelineSemaphore_GetSubmitInfo(SyrTimelineTicket* timelineTicket);

void SyrTimelineSemaphore_Destroy(SyrTimelineSemaphore* timelineSemaphore);
