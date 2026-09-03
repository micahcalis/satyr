#include "Satyr.h"
#include "Core/SatyrCore.h"
#include "Utilities/SatyrDebug.h"
#include "renderdoc_app.h"
#include <windows.h>

static const SyrConfig SYR_MAIN_CONFIG = {
    .bootupOnStartup = true,
    .pipelineCachePath = "C:/Users/micah/Desktop/Hobby/satyr/bin/pipeline_cache.bin",
    .playbackStereoEnabled = false,
    .overrideSampleRate = false,
    .overrideStandardSampleRate = 0};

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

int main()
{
#if defined(SYR_DEBUG)
    atexit(Syr_ReportMemoryLeaks);
#endif

    RENDERDOC_API_1_1_2* rdoc_api = NULL;
    SyrInitializeRenderDoc(&rdoc_api);

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

    SyrAudioAssetLoadConfig audioLoadConfig = {.filePath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/audio/Audio_TravisScott.mp3",
        .name = "AudioClearCanvas",
        .sampleRate = SYR_AUDIO_SAMPLE_RATE,
        .sampleMode = SYR_AUDIO_ASSET_SAMPLE_MODE_MONO};

    SyrAudioAsset* audioAsset = NULL;

    SyrAudioAsset_Load(&audioLoadConfig, &audioAsset);

    float notesTest = 23234.f;
    SyrChordConfig chordConfigs[4] = {
        {
            .name = "testChord",
            .notesData = {
                .name = "testNotesData",
                .size = sizeof(float)},
            .noteBufferData = &notesTest,
            .instrumentCount = 1,
            .shaderPath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/shaders/compute/TestCompute.spv",
            .kernelIndex = 0,
            .threadGroupSize = SYR_THREAD_GROUP_SIZE_L,
            .fftSize = SYR_FFT_SIZE_L,
        },
        {.name = "testChord", .notesData = {.name = "testNotesData", .size = sizeof(float)}, .noteBufferData = &notesTest, .instrumentCount = 1, .shaderPath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/shaders/compute/TestCompute.spv", .kernelIndex = 1, .threadGroupSize = SYR_THREAD_GROUP_SIZE_L, .fftSize = SYR_FFT_SIZE_L},
        {.name = "testChord", .notesData = {.name = "testNotesData", .size = sizeof(float)}, .noteBufferData = &notesTest, .instrumentCount = 1, .shaderPath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/shaders/compute/TestCompute.spv", .kernelIndex = 2, .threadGroupSize = SYR_THREAD_GROUP_SIZE_L, .fftSize = SYR_FFT_SIZE_L},
        {.name = "testChord", .notesData = {.name = "testNotesData", .size = sizeof(float)}, .noteBufferData = &notesTest, .instrumentCount = 1, .shaderPath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/shaders/compute/TestCompute.spv", .kernelIndex = 3, .threadGroupSize = SYR_THREAD_GROUP_SIZE_L, .fftSize = SYR_FFT_SIZE_L},
    };

    SyrMelodyConfig melodyConfig = {
        .name = "testMelody",
        .chordConfigs = chordConfigs,
        .chordCount = 4};

    SyrMelody* melody = SyrSyrinx_CreateMelody(syrinx, &melodyConfig);

    uint32_t testSamples = SyrAudioAsset_GetTotalSamples(audioAsset, SYR_AUDIO_ASSET_SAMPLE_MODE_MONO);
    uint32_t padding = SyrAudioBuffer_CalculatePaddingSampleCount(testSamples, SYR_FFT_SIZE_L);
    uint32_t paddedSamples = testSamples + padding;

    // 2. Calculate the exact number of threads needed for the FFT Kernels
    // (Total Chunks * Threads per Chunk)
    uint32_t totalChunks = paddedSamples / SYR_FFT_SIZE_L;
    uint32_t fftDispatchThreads = totalChunks * SYR_THREAD_GROUP_SIZE_L;

    SyrMetronomeConfig* metronomeConfig = SyrMelody_GetMetronomeConfigBody(melody);

    // Phase 0: Forward FFT (Batched workload, needs fewer threads)
    metronomeConfig->phaseConfigs[0].bindings[0].instrumentIndex = 0;
    metronomeConfig->phaseConfigs[0].bindings[0].instrumentSlot = SYR_INSTRUMENT_SLOT_0;
    metronomeConfig->phaseConfigs[0].dispatchSamples = fftDispatchThreads;
    metronomeConfig->phaseConfigs[0].requiresMaster = true;

    // // Phase 1: Filter (1-to-1 workload, dispatch all padded samples)
    metronomeConfig->phaseConfigs[1].bindings[0].instrumentIndex = 0;
    metronomeConfig->phaseConfigs[1].bindings[0].instrumentSlot = SYR_INSTRUMENT_SLOT_0;
    metronomeConfig->phaseConfigs[1].dispatchSamples = paddedSamples;
    metronomeConfig->phaseConfigs[1].requiresMaster = true;

    // Phase 2: Inverse FFT (Batched workload, needs fewer threads)
    metronomeConfig->phaseConfigs[2].bindings[0].instrumentIndex = 0;
    metronomeConfig->phaseConfigs[2].bindings[0].instrumentSlot = SYR_INSTRUMENT_SLOT_0;
    metronomeConfig->phaseConfigs[2].dispatchSamples = fftDispatchThreads;
    metronomeConfig->phaseConfigs[2].requiresMaster = true;

    // Phase 3: Copy to Master (1-to-1 workload)
    // You can use testSamples here safely so the padding doesn't get copied to the master!
    metronomeConfig->phaseConfigs[3].bindings[0].instrumentIndex = 0;
    metronomeConfig->phaseConfigs[3].bindings[0].instrumentSlot = SYR_INSTRUMENT_SLOT_0;
    metronomeConfig->phaseConfigs[3].dispatchSamples = testSamples;
    metronomeConfig->phaseConfigs[3].requiresMaster = true;

    SyrMetronome* metronome = NULL;
    SyrMetronome_Initialize(metronomeConfig, &metronome);

    SyrInstrumentConfig instrumentConfig = {
        .name = "testInstrument",
        .totalSamples = SyrAudioAsset_GetTotalSamples(audioAsset, SYR_AUDIO_ASSET_SAMPLE_MODE_MONO),
        .sampleRate = audioAsset->sampleRate,
        .sampleMode = SYR_AUDIO_ASSET_SAMPLE_MODE_MONO,
        .fftSize = SYR_FFT_SIZE_L,
        .sourceAsset = audioAsset};

    SyrSongConfig songConfig = {
        .name = "testSong",
        .instrumentConfigs = &instrumentConfig,
        .instrumentCount = 1,
        .masterSamples = testSamples,
        .masterFFTSize = SYR_FFT_SIZE_L,
        .melody = melody,
        .metronome = metronome};

    SyrSong* song = SyrSyrinx_CreateSong(syrinx, &songConfig);

    SyrProducerConfig producerConfig = {
        .name = "testProducer",
        .priority = SYR_PRODUCER_PRIORITY_HIGH};
    SyrProducer* producer = SyrRecordLabel_NewProducer(recordLabel, &producerConfig);

    SyrAlbumConfig albumConfig = {
        .name = "testAlbum"};
    SyrAlbum* album = SyrRecordLabel_NewAlbum(recordLabel, &albumConfig);

    SyrAlbum_AddSongs(album, &song, 1);
    uint64_t productionId;

    if (rdoc_api)
        rdoc_api->StartFrameCapture(NULL, NULL);

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

    SyrVinylConfig vinylConfig = {.name = "testVinyl",
        .audioAsset = discAsset->audioAssets[0],
        .mode = SYR_VINYL_MODE_LOOP,
        .ownership = SYR_VINYL_ASSET_OWNERSHIP_RELAXED};

    SyrVinylId vinylId = SyrRecordPlayer_CreateVinyl(recordPlayer, &vinylConfig);
    SyrVoiceId voiceId = SyrRecordPlayer_PlayVinyl(recordPlayer, vinylId, 2.0f, 1.0f);

    SyrAudioAssetExportConfig exportConfig = {.filePath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/audio/Audio_Test.wav",
        .sampleMode = SYR_AUDIO_ASSET_SAMPLE_MODE_MONO,
        .sampleRate = SYR_AUDIO_SAMPLE_RATE};

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
