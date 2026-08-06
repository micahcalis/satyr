#include "TimelineSemaphore.h"

typedef struct SyrTimelineSemaphore
{
    VkSemaphore semaphoreHandle;
    uint64_t ticketCounter;
    uint64_t semaphoreCounter;
    VkDevice device;
} SyrTimelineSemaphore;

SyrResult SyrTimelineSemaphore_CreateSemaphore(SyrTimelineSemaphore* timelineSemaphore)
{
    VkSemaphoreTypeCreateInfo typeCreateInfo = {0};
    typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    typeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    typeCreateInfo.initialValue = timelineSemaphore->ticketCounter;

    VkSemaphoreCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = &typeCreateInfo;

    if (vkCreateSemaphore(timelineSemaphore->device,
            &createInfo,
            NULL,
            &timelineSemaphore->semaphoreHandle)
        != VK_SUCCESS)
    {
        SYR_ERROR("Failed to create Timeline Semaphore!");
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrTimelineSemaphore_Initialize(SyrDevice* device,
    SyrTimelineSemaphore** timelineSemaphore)
{
    *timelineSemaphore = SYR_NEW(*timelineSemaphore);
    (*timelineSemaphore)->ticketCounter = 0;
    (*timelineSemaphore)->semaphoreCounter = 0;
    (*timelineSemaphore)->device = SyrDevice_GetLogicalDeviceHandle(device);

    if (SyrTimelineSemaphore_CreateSemaphore(*timelineSemaphore) != SYR_RESULT_SUCCESS)
    {
        SyrTimelineSemaphore_Destroy(*timelineSemaphore);
        *timelineSemaphore = NULL;
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrTimelineTicket SyrTimelineSemaphore_AssignTicket(SyrTimelineSemaphore* timelineSemaphore, char name[64])
{
    timelineSemaphore->ticketCounter++;

    SyrTimelineTicket ticket;
    ticket.id = timelineSemaphore->ticketCounter;

    if (name != NULL)
    {
        SYR_STR_COPY(ticket.name, name);
        ticket.name[sizeof(ticket.name) - 1] = '\0';
    } else
    {
        ticket.name[0] = '\0';
    }

    return ticket;
}

void SyrTimelineSemaphore_UpdateSemaphoreCounter(SyrTimelineSemaphore* timelineSemaphore)
{
    vkGetSemaphoreCounterValue(timelineSemaphore->device,
        timelineSemaphore->semaphoreHandle,
        &timelineSemaphore->semaphoreCounter);
}

bool SyrTimelineSemaphore_IsTicketComplete(SyrTimelineSemaphore* timelineSemaphore, const SyrTimelineTicket* ticket)
{
    return ticket->id <= timelineSemaphore->semaphoreCounter;
}

VkSemaphore SyrTimelineSemaphore_GetSemaphoreHandle(SyrTimelineSemaphore* timelineSemaphore)
{
    return timelineSemaphore->semaphoreHandle;
}

VkTimelineSemaphoreSubmitInfo SyrTimelineSemaphore_GetSubmitInfo(const SyrTimelineTicket* timelineTicket)
{
    VkTimelineSemaphoreSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    submitInfo.waitSemaphoreValueCount = 0;
    submitInfo.pWaitSemaphoreValues = NULL;
    submitInfo.signalSemaphoreValueCount = 1;
    submitInfo.pSignalSemaphoreValues = &timelineTicket->id;

    return submitInfo;
}

void SyrTimelineSemaphore_Destroy(SyrTimelineSemaphore* timelineSemaphore)
{
    if (timelineSemaphore == NULL)
        return;

    if (timelineSemaphore->semaphoreHandle != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(timelineSemaphore->device,
            timelineSemaphore->semaphoreHandle,
            NULL);
    }

    free(timelineSemaphore);
}
