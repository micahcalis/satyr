#include "Satyr.h"
#include "Core/SatyrCore.h"
#include "Utilities/SatyrDebug.h"

static const SyrConfig SYR_MAIN_CONFIG = {
    .bootupOnStartup = true,
    .pipelineCachePath = "C:/Users/micah/Desktop/Hobby/satyr/bin/pipeline_cache.bin",
    .playbackStereoEnabled = false,
    .overrideSampleRate = false,
    .overrideStandardSampleRate = 0};

int main()
{
#if defined(SYR_DEBUG)
    atexit(Syr_ReportMemoryLeaks);
#endif

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

    SyrAudioAssetLoadConfig audioLoadConfig = {.filePath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/audio/Audio_ClearCanvas.mp3",
        .name = "AudioClearCanvas",
        .sampleRate = SYR_AUDIO_SAMPLE_RATE,
        .sampleMode = SYR_AUDIO_ASSET_SAMPLE_MODE_MONO};

    SyrAudioAsset* audioAsset = NULL;

    SyrAudioAsset_Load(&audioLoadConfig, &audioAsset);

    float notesTest = 23234.f;
    SyrChordConfig chordConfig = {
        .name = "testChord",
        .notesData = {
            .name = "testNotesData",
            .size = sizeof(float)},
        .noteBufferData = &notesTest,
        .instrumentCount = 1,
        .shaderPath = "C:/Users/micah/Desktop/Hobby/satyr/bin/assets/shaders/compute/TestCompute.spv",
        .kernelIndex = 0,
        .threadGroupSize = SYR_THREAD_GROUP_SIZE_L};

    SyrMelodyConfig melodyConfig = {
        .name = "testMelody",
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

    SyrInstrumentConfig instrumentConfig = {
        .name = "testInstrument",
        .samples = SyrAudioAsset_GetTotalSamples(audioAsset, SYR_AUDIO_ASSET_SAMPLE_MODE_MONO)};

    SyrSongConfig songConfig = {
        .name = "testSong",
        .instrumentConfigs = &instrumentConfig,
        .instrumentCount = 1,
        .masterSamples = 1024,
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

    if (SyrRecordLabel_StartProduction(recordLabel, album, producer, &productionId) == SYR_RESULT_SUCCESS)
    {
        SYR_LOG("Production queued for Album: %s", SyrAlbum_GetName(album));
    }

    SyrPollEvents pollEvents;
    bool isComplete = false;

    SYR_LOG("Polling production events...");
    while (!isComplete)
    {
        if (SyrRecordLabel_PollEvents(recordLabel, &pollEvents) == SYR_RESULT_SUCCESS)
        {
            for (uint32_t i = 0; i < pollEvents.count; i++)
            {
                const SyrProductionEvent* event = &pollEvents.events[i];

                if (event->state == SYR_PRODUCTION_STATE_RECORDED)
                {
                    SYR_LOG("Album '%s' finished recording on GPU timeline!",
                        SyrAlbum_GetName(event->production.album));

                    isComplete = true;
                }
            }
        }
    }

    uint64_t halfSegment = audioAsset->totalFrames / 10;

    SyrVinylConfig vinylConfig = {.name = "testVinyl",
        .audioAsset = audioAsset,
        .mode = SYR_VINYL_MODE_LOOP_SEGMENT,
        .ownership = SYR_VINYL_ASSET_OWNERSHIP_STRICT,
        .frameSegmentBegin = 0,
        .frameSegmentEnd = halfSegment};

    SyrVinylId vinylId = SyrRecordPlayer_CreateVinyl(recordPlayer, &vinylConfig);
    SyrRecordPlayer_PlayVinyl(recordPlayer, vinylId, 1.0f, 0.8f);

    SYR_LOG("Press 'Enter' to Exit...");
    getchar();

    SyrDevice_WaitIdle(SyrSyrinx_GetDevice(syrinx));

    SyrRecordPlayer_Destroy(recordPlayer);

    SyrMelody_Destroy(melody);
    SyrMetronome_Destroy(metronome);
    // SyrAudioAsset_Destroy(audioAsset);

    SyrRecordLabel_Destroy(recordLabel);
    SyrSyrinx_Destroy(syrinx);

    return SYR_RESULT_SUCCESS;
}
