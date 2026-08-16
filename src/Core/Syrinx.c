#include "Syrinx.h"
#include "SatyrCore.h"
#include "Vulkan/Device.h"
#include "Vulkan/VulkInstance.h"
#include "Vulkan/Allocator.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/TimelineSemaphore.h"
#include "System/RecordLabel//Instrument.h"

typedef struct SyrSyrinx
{
    SyrVulkInstance* vulkInstance;
    SyrDevice* device;
    SyrAllocator* allocator;
    SyrPipelineCache* pipelineCache;
} SyrSyrinx;

static SyrResult SyrSyrinx_InitializeVulkan(SyrSyrinx* syrinx, const SyrConfig* config)
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

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrSyrinx_Initialize(const SyrConfig* config,
    SyrSyrinx** syrinx)
{
    *syrinx = SYR_NEW(*syrinx);

    if (SyrSyrinx_InitializeVulkan(*syrinx, config) != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to initialize Vulkan (Syrinx)!");
        SyrSyrinx_Destroy(*syrinx);
        *syrinx = NULL;
        return SYR_RESULT_VULKAN_FAILED;
    }

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
    uint32_t ssboCount = SYR_MASTER_SSBO_COUNT + config->instrumentCount * SYR_INSTRUMENT_SSBO_COUNT;
    SyrDescriptor* descriptor = SyrAllocator_AllocateDescriptor(ssboCount,
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
            config->instrumentCount,
            config->threadGroupSize,
            &chord)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to create Chord: %s!", config->name);
        SyrNoteBuffer_Destroy(noteBuffer);
        SyrPipeline_Destroy(pipeline);
        SyrDescriptor_Destroy(descriptor);
        return NULL;
    }

    if (config->noteBufferData != NULL)
    {
        SyrChord_WriteNotes(chord, config->noteBufferData, config->notesData.size, 0);
    }

    return chord;
}

SyrMelody* SyrSyrinx_CreateMelody(SyrSyrinx* syrinx,
    const SyrMelodyConfig* config)
{
    SyrMelody* melody = NULL;

    if (SyrMelody_Initialize(config->name, &melody) != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to create Melody: %s!", config->name);
        return NULL;
    }

    SyrChord* chords[SYR_MAX_CHORDS] = {0};

    for (uint32_t i = 0; i < config->chordCount; i++)
    {
        chords[i] = SyrSyrinx_CreateChord(syrinx, &config->chordConfigs[i]);

        if (chords[i] == NULL)
        {
            SYR_ERROR("Failed to create Chord at index %u for Melody: %s!", i, config->name);

            for (uint32_t j = 0; j < i; j++)
            {
                SyrChord_Destroy(chords[j]);
            }

            SyrMelody_Destroy(melody);
            return NULL;
        }
    }

    SyrMelody_AddChords(melody, chords, config->chordCount);

    return melody;
}

SyrProducer* SyrSyrinx_CreateProducer(SyrSyrinx* syrinx,
    const SyrProducerConfig* config)
{
    SyrTimelineSemaphore* timelineSemaphore = NULL;

    if (SyrTimelineSemaphore_Initialize(syrinx->device, &timelineSemaphore) != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to create Timeline Semaphore for Producer: %s!", config->name);
        return NULL;
    }

    SyrCommandPool* commandPool = NULL;

    if (SyrCommandPool_Initialize(SyrDevice_GetLogicalDeviceHandle(syrinx->device),
            SyrDevice_GetComputeQueue(syrinx->device, (SyrQueuePriorityLevel)config->priority),
            SyrDevice_GetComputeFamilyIndex(syrinx->device),
            &commandPool)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to create Command Pool for Producer: %s!", config->name);
        SyrTimelineSemaphore_Destroy(timelineSemaphore);
        return NULL;
    }

    SyrProducer* producer = NULL;

    if (SyrProducer_Initialize(commandPool,
            timelineSemaphore,
            config->priority,
            config->name,
            &producer)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to create Producer: %s!", config->name);
        return NULL;
    }

    return producer;
}

SyrInstrument* SyrSyrinx_CreateInstrument(SyrSyrinx* syrinx,
    const SyrInstrumentConfig* config)
{
    SyrInstrument* instrument = NULL;

    if (SyrInstrument_Initialize(config->samples,
            config->name,
            syrinx->allocator,
            &instrument)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to create Instrument: %s", config->name);
        return NULL;
    }

    return instrument;
}

SyrAudioBuffer* SyrSyrinx_CreateMasterBuffer(SyrSyrinx* syrinx,
    const uint32_t masterSamples)
{
    SyrAudioBuffer* masterBuffer = NULL;

    if (SyrAudioBuffer_Initialize(masterSamples,
            syrinx->allocator,
            &masterBuffer)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to create Master Buffer with samples: %u", masterSamples);
        return NULL;
    }

    return masterBuffer;
}

SyrSong* SyrSyrinx_CreateSong(SyrSyrinx* syrinx,
    const SyrSongConfig* config)
{
    if (config->melody == NULL)
    {
        SYR_ERROR("Can't create Song (name: %s) with null Melody!", config->name);
        return NULL;
    }

    SyrAudioBuffer* masterBuffer = SyrSyrinx_CreateMasterBuffer(syrinx, config->masterSamples);

    if (masterBuffer == NULL)
    {
        SYR_ERROR("Failed to create Master Buffer for Song: %s", config->name);
        return NULL;
    }

    SyrSong* song = NULL;

    if (SyrSong_Initialize(masterBuffer,
            config->melody,
            config->metronome,
            config->name,
            &song)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to create Song: %s", config->name);
        return NULL;
    }

    SyrInstrument* instruments[SYR_MAX_INSTRUMENTS] = {0};

    for (uint32_t i = 0; i < config->instrumentCount; i++)
    {
        instruments[i] = SyrSyrinx_CreateInstrument(syrinx, &config->instrumentConfigs[i]);

        if (instruments[i] == NULL)
        {
            SYR_ERROR("Failed to create Instrument (name: %s) for Song: %s",
                config->instrumentConfigs[i].name,
                config->name);

            for (uint32_t j = 0; j < i; j++)
            {
                SyrInstrument_Destroy(instruments[j]);
            }

            SyrSong_Destroy(song);
            return NULL;
        }
    }

    SyrSong_AddInstruments(song, instruments, config->instrumentCount);

    return song;
}

SyrAlbum* SyrSyrinx_CreateAlbum(SyrSyrinx* syrinx,
    const SyrAlbumConfig* config)
{
    SyrAlbum* album = NULL;

    if (SyrAlbum_Initialize(config->name,
            &album)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to create Album: %s", config->name);
        return NULL;
    }

    return album;
}

SyrDevice* SyrSyrinx_GetDevice(const SyrSyrinx* syrinx)
{
    return syrinx->device;
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

    SyrDevice_WaitIdle(syrinx->device);
    SyrSyrinx_CleanupVulkan(syrinx);
    SYR_FREE(syrinx);
}
