#include "Syrinx.h"
#include "SatyrCore.h"
#include "Vulkan/Device.h"
#include "Vulkan/VulkInstance.h"
#include "Vulkan/Allocator.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/TimelineSemaphore.h"

typedef struct SyrSyrinx
{
    SyrVulkInstance* vulkInstance;
    SyrDevice* device;
    SyrAllocator* allocator;
    SyrPipelineCache* pipelineCache;
} SyrSyrinx;

SyrSyrinx* SyrSyrinx_Create(const SyrConfig* config)
{
    SyrSyrinx* syrinx = SYR_ALLOC(SyrSyrinx);
    return syrinx;
}

SyrResult SyrSyrinx_InitializeVulkan(SyrSyrinx* syrinx, const SyrConfig* config)
{
    if (SyrVulkInstance_Initialize(config, &syrinx->vulkInstance) == SYR_RESULT_VULKAN_FAILED)
    {
        return SYR_RESULT_VULKAN_FAILED;
    }

    if (SyrDevice_Initialize(config, syrinx->vulkInstance, &syrinx->device) == SYR_RESULT_VULKAN_FAILED)
    {
        return SYR_RESULT_VULKAN_FAILED;
    }

    if (SyrAllocator_Initialize(config, syrinx->vulkInstance, syrinx->device, &syrinx->allocator) == SYR_RESULT_VULKAN_FAILED)
    {
        return SYR_RESULT_VULKAN_FAILED;
    }

    syrinx->pipelineCache = NULL;

#ifdef SYR_ENABLE_VULKAN_PIPELINE_CACHE
    if (config->pipelineCachePath != NULL)
    {
        if (SyrPipelineCache_Initialize(config, syrinx->device, &syrinx->pipelineCache) == SYR_RESULT_VULKAN_FAILED)
        {
            return SYR_RESULT_VULKAN_FAILED;
        }
    }
#endif

    // SyrBufferAllocParams allocParams = {0};
    // allocParams.createFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    //     | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    // allocParams.memoryFlags = VMA_MEMORY_USAGE_AUTO;
    // allocParams.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    // allocParams.size = 128;

    // SyrBufferAllocation* bufferAllocation = SyrAllocator_AllocateBuffer(allocParams,
    //     syrinx->allocator);

    // SyrDescriptor* descriptor = SyrAllocator_AllocateDescriptor(8,
    //     syrinx->allocator);

    // SyrPipeline* pipeline = NULL;

    // if (SyrPipeline_Initialize("C:/Users/micah/Desktop/Hobby/satyr/bin/assets/shaders/compute/TestCompute.spv",
    //         0,
    //         SyrDescriptor_GetLayout(descriptor),
    //         syrinx->device,
    //         syrinx->pipelineCache,
    //         &pipeline)
    //     != SYR_RESULT_SUCCESS)
    // {
    //     return SYR_RESULT_VULKAN_FAILED;
    // }

    // SyrPipeline_Destroy(pipeline);
    // SyrDescriptor_Destroy(descriptor);

    // SyrShaderModule* shaderModule = NULL;

    // if (SyrShaderModule_Initialize("C:/Users/micah/Desktop/Hobby/satyr/bin/assets/shaders/compute/TestCompute.spv",
    //         SyrDevice_GetLogicalDeviceHandle(syrinx->device),
    //         &shaderModule)
    //     != SYR_RESULT_SUCCESS)
    // {
    //     return SYR_RESULT_RUNTIME_ERROR;
    // }

    // SyrShaderModule_Destroy(shaderModule);

    // SyrTimelineSemaphore* timelineSemaphore = NULL;
    // SyrTimelineSemaphore_Initialize(syrinx->device, &timelineSemaphore);
    // SyrTimelineTicket ticket = SyrTimelineSemaphore_AssignTicket(timelineSemaphore, "testTicket");

    // SyrCommandBuffer* commandBuffer = SyrAllocator_AllocateCommandBuffer(syrinx->allocator);

    // SyrBarrier barrier = SyrBarrier_Initialize(SYR_RESOURCE_ACTION_UNDEFINED,
    //     SYR_RESOURCE_ACTION_BUFFER_READ_WRITE,
    //     bufferAllocation);

    // SyrCommandBuffer_Begin(commandBuffer);
    // SyrCommandBuffer_RecordBarrier(commandBuffer, barrier);
    // SyrCommandBuffer_EndSubmit(commandBuffer, timelineSemaphore, &ticket);

    // SyrDevice_WaitIdle(syrinx->device);

    // SyrCommandBuffer_Destroy(commandBuffer);
    // SyrBufferAllocation_Destroy(bufferAllocation);

    // SyrTimelineSemaphore_UpdateSemaphoreCounter(timelineSemaphore);

    // bool isComplete = SyrTimelineSemaphore_IsTicketComplete(timelineSemaphore, &ticket);
    // SYR_LOG("Timeline Ticket (%s) Status: %d",
    //     ticket.name,
    //     (int)isComplete);

    // SyrTimelineSemaphore_Destroy(timelineSemaphore);

    SyrChordConfig chordConfig = {.name = "testChord",
        .notesData = {
            .name = "testNotesData",
            .size = sizeof(float)},
        .instrumentCount = 1,
        .shaderPath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/shaders/compute/TestCompute.spv",
        .kernelIndex = 0};

    SyrChord* chord = SyrSyrinx_CreateChord(syrinx, &chordConfig);
    float notesTest = 23234.f;
    SyrChord_WriteNotes(chord, &notesTest, sizeof(float), 0);

    SyrChord_Destroy(chord);

    return SYR_RESULT_SUCCESS;
}

SyrNoteBuffer* SyrSyrinx_CreateNoteBuffer(SyrSyrinx* syrinx,
    const SyrNotesData notesData)
{
    SyrBufferAllocParams allocParams = {0};
    allocParams.createFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocParams.memoryFlags = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocParams.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    allocParams.size = notesData.size;

    SyrBufferAllocation* bufferAllocation = SyrAllocator_AllocateBuffer(allocParams, syrinx->allocator);

    SyrNoteBuffer* noteBuffer = NULL;

    if (SyrNoteBuffer_Initialize(notesData,
            bufferAllocation,
            &noteBuffer)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Syrinx failed to create Note Buffer: %s", notesData.name);
        return NULL;
    }

    return noteBuffer;
}

SyrChord* SyrSyrinx_CreateChord(SyrSyrinx* syrinx,
    const SyrChordConfig* config)
{
    SyrDescriptor* descriptor = SyrAllocator_AllocateDescriptor(config->instrumentCount,
        syrinx->allocator);

    if (descriptor == NULL)
    {
        SYR_ERROR("Failed to create Chord Descriptor: %s!", config->name);
        return NULL;
    }

    SyrPipeline* pipeline = NULL;

    if (SyrPipeline_Initialize(config->shaderPath,
            config->kernelIndex,
            SyrDescriptor_GetLayout(descriptor),
            syrinx->device,
            syrinx->pipelineCache,
            &pipeline)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to create Chord Pipeline: %s!", config->name);
        SyrDescriptor_Destroy(descriptor);
        return NULL;
    }

    SyrNoteBuffer* noteBuffer = SyrSyrinx_CreateNoteBuffer(syrinx, config->notesData);

    if (noteBuffer == NULL)
    {
        SYR_ERROR("Failed to create Chord Note Buffer: %s!", config->name);
        SyrPipeline_Destroy(pipeline);
        SyrDescriptor_Destroy(descriptor);
        return NULL;
    }

    SyrChord* chord = NULL;

    if (SyrChord_Initialize(pipeline,
            descriptor,
            noteBuffer,
            config->name,
            &chord)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to create Chord: %s!", config->name);
        SyrNoteBuffer_Destroy(noteBuffer);
        SyrPipeline_Destroy(pipeline);
        SyrDescriptor_Destroy(descriptor);
        return NULL;
    }

    return chord;
}

static void SyrSyrinx_CleanupVulkan(SyrSyrinx* syrinx)
{
    SyrPipelineCache_Destroy(syrinx->pipelineCache);
    SyrAllocator_Destroy(syrinx->allocator);
    SyrDevice_Destroy(syrinx->device);
    SyrVulkInstance_Destroy(syrinx->vulkInstance);
}

void SyrSyrinx_Destroy(SyrSyrinx* syrinx)
{
    if (syrinx == NULL)
        return;

    SyrSyrinx_CleanupVulkan(syrinx);
    free(syrinx);
}
