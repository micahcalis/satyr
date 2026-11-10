#include "Satyr.h"
#include "Core/SatyrCore.h"
#include "System/RecordLabel/Chord.h"
#include "Utilities/SatyrDebug.h"
#include "renderdoc_app.h"
#include <windows.h>
#include "NGFX_GPUTrace_Vulkan.h"

static const SyrConfig SYR_MAIN_CONFIG = {
    .bootupOnStartup = true,
    .pipelineCachePath = "C:/Users/micah/Desktop/Hobby/satyr/bin/pipeline_cache.bin",
    .playbackStereoEnabled = false,
    .overrideSampleRate = false,
    .overrideStandardSampleRate = 0
};

static void SyrInitializeRenderDoc(RENDERDOC_API_1_1_2** rdoc_api)
{
    HMODULE mod = GetModuleHandleA("renderdoc.dll");
    if (mod)
    {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&(*rdoc_api));
        if (ret != 1)
            *rdoc_api = NULL;
    }
}

static SyrResult SyrInitializeNsight()
{
    NGFX_SetLibraryLoadFn(NGFX_LoadLib_NoVerification);

    NGFX_GPUTrace_InitializeActivity_Vulkan_Params params = { NGFX_GPUTrace_InitializeActivity_Vulkan_Params_VER };

    if (NGFX_GPUTrace_InitializeActivity_Vulkan(&params) != NGFX_Result_Success)
    {
        SYR_LOG("Nsight Initialization not successful");
        return SYR_RESULT_FAILED;
    }

    SYR_LOG("Nsight Initialization is successful");
    return SYR_RESULT_SUCCESS;
}

static void SyrNsightStartTrace(const SyrResult initializationResult)
{
    if (initializationResult == SYR_RESULT_FAILED)
    {
        return;
    }

    NGFX_GPUTrace_StartTrace_Vulkan_Params_V1 params = { NGFX_GPUTrace_StartTrace_Vulkan_Params_VER };
    NGFX_GPUTrace_StartTrace_Vulkan(&params);
}

static void SyrNsightEndTrace(const SyrResult initializationResult)
{
    if (initializationResult == SYR_RESULT_FAILED)
    {
        return;
    }

    NGFX_GPUTrace_StopTrace_Vulkan_Params_V1 params = { NGFX_GPUTrace_StopTrace_Vulkan_Params_VER };
    NGFX_GPUTrace_StopTrace_Vulkan(&params);
}

int main()
{
#if defined(SYR_DEBUG)
#endif

    RENDERDOC_API_1_1_2* rdoc_api = NULL;
    SyrInitializeRenderDoc(&rdoc_api);
    SyrResult nsightInitialized = SyrInitializeNsight();

    SYR_LOG("Press 'Enter' to Start..");

    SyrSyrinx* syrinx = NULL;
    if (SyrSyrinx_Initialize(&SYR_MAIN_CONFIG, &syrinx) != SYR_RESULT_SUCCESS)
    {
        return SYR_RESULT_FAILED;
    }

    SyrRecordLabel* recordLabel = NULL;
    if (SyrRecordLabel_Initialize(syrinx, &recordLabel) != SYR_RESULT_SUCCESS)
    {
        SyrSyrinx_Destroy(syrinx);
        return SYR_RESULT_FAILED;
    }

    SyrRecordPlayer* recordPlayer = NULL;
    if (SyrRecordPlayer_Initialize(&SYR_MAIN_CONFIG, &recordPlayer) != SYR_RESULT_SUCCESS)
    {
        SyrRecordLabel_Destroy(recordLabel);
        SyrSyrinx_Destroy(syrinx);
        return SYR_RESULT_FAILED;
    }

    SyrAudioAssetLoadConfig audioLoadConfig = { .filePath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/audio/Audio_Denzel.mp3",
        .name = "AudioClearCanvas",
        .sampleRate = SYR_AUDIO_SAMPLE_RATE,
        .sampleMode = SYR_AUDIO_ASSET_SAMPLE_MODE_MONO };

    SyrAudioAsset* audioAsset = NULL;

    SyrAudioAsset_Load(&audioLoadConfig, &audioAsset);

#define CHORD_COUNT 5

    SyrWOLAMode wolaMode = SYR_WOLA_MODE_HANN;

    float notesTest = 23234.f;
    SyrChordConfig chordConfigs[CHORD_COUNT] = {
        { .name = "testChord",
            .notesData = {
                .name = "testNotesData",
                .size = sizeof(float) },
            .noteBufferData = &notesTest,
            .instrumentCount = 1,
            .shaderPath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/shaders/compute/TestCompute.spv",
            .kernelIndex = 0,
            .threadGroupSize = SYR_THREAD_GROUP_SIZE_L,
            .fftSize = SYR_FFT_SIZE_L,
            .dispatchMode = SYR_DISPATCH_MODE_FFT,
            .wolaMode = wolaMode },
        { .name = "testChord", .notesData = { .name = "testNotesData", .size = sizeof(float) }, .noteBufferData = &notesTest, .instrumentCount = 1, .shaderPath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/shaders/compute/TestCompute.spv", .kernelIndex = 1, .threadGroupSize = SYR_THREAD_GROUP_SIZE_L, .fftSize = SYR_FFT_SIZE_L, .dispatchMode = SYR_DISPATCH_MODE_FFT, .wolaMode = wolaMode },
        { .name = "testChord", .notesData = { .name = "testNotesData", .size = sizeof(float) }, .noteBufferData = &notesTest, .instrumentCount = 1, .shaderPath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/shaders/compute/TestCompute.spv", .kernelIndex = 2, .threadGroupSize = SYR_THREAD_GROUP_SIZE_L, .fftSize = SYR_FFT_SIZE_L, .dispatchMode = SYR_DISPATCH_MODE_FFT, .wolaMode = wolaMode },
        { .name = "testChord", .notesData = { .name = "testNotesData", .size = sizeof(float) }, .noteBufferData = &notesTest, .instrumentCount = 1, .shaderPath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/shaders/compute/TestCompute.spv", .kernelIndex = 3, .threadGroupSize = SYR_THREAD_GROUP_SIZE_L, .fftSize = SYR_FFT_SIZE_L, .dispatchMode = SYR_DISPATCH_MODE_FFT, .wolaMode = wolaMode },
        { .name = "testChord", .notesData = { .name = "testNotesData", .size = sizeof(float) }, .noteBufferData = &notesTest, .instrumentCount = 1, .shaderPath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/shaders/compute/TestCompute.spv", .kernelIndex = 4, .threadGroupSize = SYR_THREAD_GROUP_SIZE_L, .fftSize = SYR_FFT_SIZE_L, .dispatchMode = SYR_DISPATCH_MODE_LINEAR, .wolaMode = wolaMode }
    };

    SyrMelodyConfig melodyConfig = {
        .name = "testMelody",
        .chordConfigs = chordConfigs,
        .chordCount = CHORD_COUNT
    };

    SyrMelody* melody = SyrSyrinx_CreateMelody(syrinx, &melodyConfig);

    uint32_t testSamples = SyrAudioAsset_GetTotalSamples(audioAsset, SYR_AUDIO_ASSET_SAMPLE_MODE_MONO);
    uint32_t padding = SyrAudioBuffer_CalculatePaddingSampleCount(testSamples, SYR_FFT_SIZE_L);
    uint32_t paddedSamples = testSamples + padding;

    SyrMetronomeConfig* metronomeConfig = SyrMelody_GetMetronomeConfigBody(melody);

    metronomeConfig->phaseConfigs[0].bindings[0].instrumentIndex = 0;
    metronomeConfig->phaseConfigs[0].bindings[0].instrumentSlot = SYR_INSTRUMENT_SLOT_0;
    metronomeConfig->phaseConfigs[0].dispatchSamples = paddedSamples;
    metronomeConfig->phaseConfigs[0].requiresMaster = true;

    metronomeConfig->phaseConfigs[1].bindings[0].instrumentIndex = 0;
    metronomeConfig->phaseConfigs[1].bindings[0].instrumentSlot = SYR_INSTRUMENT_SLOT_0;
    metronomeConfig->phaseConfigs[1].dispatchSamples = paddedSamples;
    metronomeConfig->phaseConfigs[1].requiresMaster = true;

    metronomeConfig->phaseConfigs[2].bindings[0].instrumentIndex = 0;
    metronomeConfig->phaseConfigs[2].bindings[0].instrumentSlot = SYR_INSTRUMENT_SLOT_0;
    metronomeConfig->phaseConfigs[2].dispatchSamples = paddedSamples;
    metronomeConfig->phaseConfigs[2].requiresMaster = true;

    metronomeConfig->phaseConfigs[3].bindings[0].instrumentIndex = 0;
    metronomeConfig->phaseConfigs[3].bindings[0].instrumentSlot = SYR_INSTRUMENT_SLOT_0;
    metronomeConfig->phaseConfigs[3].dispatchSamples = paddedSamples;
    metronomeConfig->phaseConfigs[3].requiresMaster = true;

    metronomeConfig->phaseConfigs[4].bindings[0].instrumentIndex = 0;
    metronomeConfig->phaseConfigs[4].bindings[0].instrumentSlot = SYR_INSTRUMENT_SLOT_0;
    metronomeConfig->phaseConfigs[4].dispatchSamples = paddedSamples;
    metronomeConfig->phaseConfigs[4].requiresMaster = true;

    SyrMetronome* metronome = NULL;
    SyrMetronome_Initialize(metronomeConfig, &metronome);

    SyrInstrumentConfig instrumentConfig = {
        .name = "testInstrument",
        .totalSamples = SyrAudioAsset_GetTotalSamples(audioAsset, SYR_AUDIO_ASSET_SAMPLE_MODE_MONO),
        .sampleRate = audioAsset->sampleRate,
        .sampleMode = SYR_AUDIO_ASSET_SAMPLE_MODE_MONO,
        .fftSize = SYR_FFT_SIZE_L,
        .sourceAsset = audioAsset
    };

    SyrSongConfig songConfig = {
        .name = "testSong",
        .instrumentConfigs = &instrumentConfig,
        .instrumentCount = 1,
        .masterSamples = testSamples,
        .masterFFTSize = SYR_FFT_SIZE_L,
        .melody = melody,
        .metronome = metronome
    };

    SyrSong* song = SyrSyrinx_CreateSong(syrinx, &songConfig);

    SyrProducerConfig producerConfig = {
        .name = "testProducer",
        .priority = SYR_PRODUCER_PRIORITY_HIGH
    };
    SyrProducer* producer = SyrRecordLabel_NewProducer(recordLabel, &producerConfig);

    SyrAlbumConfig albumConfig = {
        .name = "testAlbum"
    };
    SyrAlbum* album = SyrRecordLabel_NewAlbum(recordLabel, &albumConfig);

    SyrAlbum_AddSongs(album, &song, 1);
    uint64_t productionId;

    if (rdoc_api)
        rdoc_api->StartFrameCapture(NULL, NULL);

    SyrNsightStartTrace(nsightInitialized);

    if (SyrRecordLabel_StartProduction(recordLabel, album, producer, SYR_PRODUCTION_TYPE_RECORD_RELEASE, &productionId) == SYR_RESULT_SUCCESS)
    {
        SYR_LOG("Production queued for Album: %s", SyrAlbum_GetName(album));
    }

    SyrPollEvents pollEvents;
    bool isComplete = false;
    SyrDiscAsset* discAsset = NULL;

    SYR_LOG("Polling production events...");
    while (!isComplete)
    {
        if (SyrRecordLabel_PollEvents(recordLabel, &pollEvents) == SYR_RESULT_SUCCESS)
        {
            for (uint32_t i = 0; i < pollEvents.count; i++)
            {
                const SyrProductionEvent* event = &pollEvents.events[i];

                if (event->state == SYR_PRODUCTION_STATE_RELEASED)
                {
                    SYR_LOG("Album '%s' finished recording on GPU timeline!",
                        SyrAlbum_GetName(event->production.album));

                    isComplete = true;
                    discAsset = event->production.discAsset;
                }
            }
        }
    }

    if (rdoc_api)
        rdoc_api->EndFrameCapture(NULL, NULL);

    SyrNsightEndTrace(nsightInitialized);

    SyrVinylConfig vinylConfig = { .name = "testVinyl",
        .audioAsset = discAsset->audioAssets[0],
        .mode = SYR_VINYL_MODE_LOOP,
        .ownership = SYR_VINYL_ASSET_OWNERSHIP_RELAXED };

    SyrVinylId vinylId = SyrRecordPlayer_CreateVinyl(recordPlayer, &vinylConfig);
    SyrVoiceId voiceId = SyrRecordPlayer_PlayVinyl(recordPlayer, vinylId, 1.0f, 1.0f);

    SyrAudioAssetExportConfig exportConfig = { .filePath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/audio/Audio_Test.wav",
        .sampleMode = SYR_AUDIO_ASSET_SAMPLE_MODE_MONO,
        .sampleRate = SYR_AUDIO_SAMPLE_RATE };

    SyrAudioAsset_ExportWAV(discAsset->audioAssets[0], &exportConfig);

    SYR_LOG("Press 'Enter' to Stop Voice...");
    getchar();

    SyrRecordPlayer_StopVoice(recordPlayer, voiceId);

    SYR_LOG("Press 'Enter' to Exit...");
    getchar();

    SyrDevice_WaitIdle(SyrSyrinx_GetDevice(syrinx));

    SyrRecordPlayer_Destroy(recordPlayer);

    SyrMelody_Destroy(melody);
    SyrMetronome_Destroy(metronome);
    SyrDiscAsset_Destroy(discAsset, true);
    SyrAudioAsset_Destroy(audioAsset);

    SyrRecordLabel_Destroy(recordLabel);
    SyrSyrinx_Destroy(syrinx);

    return SYR_RESULT_SUCCESS;
}
