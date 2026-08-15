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
    SyrCommandBuffer* commandBuffer;
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

    syrinx->commandBuffer = SyrAllocator_AllocateCommandBuffer(syrinx->allocator);

    float notesTest = 23234.f;

    SyrAudioAsset* audioAsset = NULL;

    SyrAudioAsset_LoadWAV("C:/Users/micah/Desktop/Hobby/satyr/bin/assets/audio/Audio_Paint.wav",
        "AudioPaint",
        &audioAsset);

    SyrChordConfig chordConfig = {.name = "testChord",
        .notesData = {
            .name = "testNotesData",
            .size = sizeof(float)},
        .noteBufferData = &notesTest,
        .instrumentCount = 1,
        .shaderPath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/shaders/compute/TestCompute.spv",
        .kernelIndex = 0,
        .threadGroupSize = SYR_THREAD_GROUP_SIZE_L};

    SyrMelodyConfig melodyConfig = {.name = "testMelody",
        .chordConfigs = &chordConfig,
        .chordCount = 1};

    SyrMelody* melody = SyrSyrinx_CreateMelody(syrinx, &melodyConfig);

    SyrMetronomeConfig* metronomeConfig = SyrMelody_GetMetronomeConfigBody(melody);
    metronomeConfig->phaseConfigs[0].bindings[0].instrumentIndex = 0;
    metronomeConfig->phaseConfigs[0].bindings[0].instrumentSlot = SYR_INSTRUMENT_SLOT_0;
    metronomeConfig->phaseConfigs[0].dispatchSamples = SyrAudioAsset_GetTotalSamples(audioAsset, SYR_AUDIO_ASSET_SAMPLE_MODE_MONO);
    metronomeConfig->phaseConfigs[0].requiresMaster = true;

    SyrMetronome* metronome = NULL;
    SyrMetronome_Initialize(metronomeConfig, &metronome);

    SyrInstrumentConfig instrumentConfig = {.name = "testInstrument",
        .samples = SyrAudioAsset_GetTotalSamples(audioAsset, SYR_AUDIO_ASSET_SAMPLE_MODE_MONO)};

    SyrSongConfig songConfig = {.name = "testSong",
        .instrumentConfigs = &instrumentConfig,
        .instrumentCount = 1,
        .masterSamples = 1024,
        .melody = melody,
        .metronome = metronome};

    SyrProducerConfig producerConfig = {.name = "testProducer"};
    SyrProducer* producer = SyrSyrinx_CreateProducer(syrinx, &producerConfig);

    SyrAlbumConfig albumConfig = {.name = "testAlbum"};
    SyrAlbum* album = SyrSyrinx_CreateAlbum(syrinx, &albumConfig);

    SyrSong* song = SyrSyrinx_CreateSong(syrinx, &songConfig);

    SyrAlbum_AddSongs(album, &song, 1);
    const SyrTimelineTicket* ticket = NULL;
    SyrAlbum_RecordSongs(album, producer, &ticket);

    SyrDevice_WaitIdle(syrinx->device);

    SyrAlbum_Destroy(album);
    SyrProducer_Destroy(producer);
    SyrMelody_Destroy(melody);
    SyrMetronome_Destroy(metronome);
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
            syrinx->commandBuffer,
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

    if (SyrAlbum_Initialize(syrinx->commandBuffer,
            config->name,
            &album)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to create Album: %s", config->name);
        return NULL;
    }

    return album;
}

static void SyrSyrinx_CleanupVulkan(SyrSyrinx* syrinx)
{
    SyrCommandBuffer_Destroy(syrinx->commandBuffer);
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
    free(syrinx);
}
