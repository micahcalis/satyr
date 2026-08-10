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

    float notesTest = 23234.f;

    SyrChordConfig chordConfig = {.name = "testChord",
        .notesData = {
            .name = "testNotesData",
            .size = sizeof(float)},
        .noteBufferData = &notesTest,
        .instrumentCount = 1,
        .shaderPath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/shaders/compute/TestCompute.spv",
        .kernelIndex = 0};

    uint32_t chordCount = SYR_MAX_CHORDS;
    SyrChordConfig chordConfigs[SYR_MAX_CHORDS];

    for (uint32_t i = 0; i < SYR_MAX_CHORDS; i++)
    {
        chordConfigs[i] = chordConfig;
        snprintf(chordConfigs[i].name, sizeof(chordConfigs[i].name), "testChords%u", i);
    }

    SyrMelodyConfig melodyConfig = {.name = "testMelody",
        .chordConfigs = chordConfigs,
        .chordCount = chordCount};

    SyrMelody* melody = SyrSyrinx_CreateMelody(syrinx, &melodyConfig);
    SyrMelody_PrintChords(melody);
    SyrMelody_Destroy(melody);

    SyrProducerConfig producerConfig = {.name = "testProducer"};
    SyrProducer* producer = SyrSyrinx_CreateProducer(syrinx, &producerConfig);

    SyrProducer_Destroy(producer);

    SyrAudioAsset* audioAsset = NULL;

    SyrAudioAsset_LoadMP3("C:/Users/micah/Desktop/Hobby/satyr/bin/assets/audio/Audio_ClearCanvas.mp3",
        "testclip",
        &audioAsset);

    SyrInstrumentConfig instrumentConfig = {.name = "testInstrument",
        .samples = SyrAudioAsset_GetTotalSamples(audioAsset, SYR_AUDIO_ASSET_SAMPLE_MODE_MONO)};

    SyrInstrument* instrument = SyrSyrinx_CreateInstrument(syrinx, &instrumentConfig);

    SyrInstrument_UploadAsset(instrument, audioAsset);

    SyrInstrument_Destroy(instrument);
    SyrAudioAsset_Destroy(audioAsset);

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
    uint32_t ssboCount = config->instrumentCount * SYR_INSTRUMENT_SSBO_COUNT;
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

    SyrProducer* producer = NULL;

    if (SyrProducer_Initialize(timelineSemaphore, config->name, &producer) != SYR_RESULT_SUCCESS)
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
