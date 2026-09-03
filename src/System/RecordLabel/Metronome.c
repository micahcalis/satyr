#include "Metronome.h"

typedef struct SyrMetronome
{
    SyrList(SyrMetronomePhase) phases;
} SyrMetronome;

SyrResult SyrMetronome_Initialize(SyrMetronomeConfig* config,
    SyrMetronome** metronome)
{
    *metronome = SYR_NEW(*metronome);
    (*metronome)->phases = NULL;

    return SyrMetronome_Configure(*metronome, config);
}

static void SyrMetronome_AddBindingToPhase(SyrMetronomePhase* phase,
    const SyrInstrumentBinding binding)
{
    SyrList_Push(phase->instrumentBindings, binding);
}

SyrMetronomePhase* SyrMetronome_AddPhase(SyrMetronome* metronome,
    const SyrMetronomePhaseConfig* config)
{
    if (config->bindingCount != SyrList_Count(config->bindings))
    {
        SYR_ERROR("Metronome Phase Configuration Bindings Count mismatch!");
        return NULL;
    }

    SyrMetronomePhase phase = {.instrumentBindings = NULL,
        .instrumentCount = config->bindingCount,
        .requiresMaster = config->requiresMaster,
        .dispatchSamples = config->dispatchSamples};

    for (size_t i = 0; i < phase.instrumentCount; i++)
    {
        SyrMetronome_AddBindingToPhase(&phase, config->bindings[i]);
    }

    SyrList_Push(metronome->phases, phase);
    return &metronome->phases[SyrList_Count(metronome->phases) - 1];
}

SyrResult SyrMetronome_Configure(SyrMetronome* metronome,
    SyrMetronomeConfig* config)
{
    if (config->phaseCount != SyrList_Count(config->phaseConfigs))
    {
        SYR_ERROR("Metronome Configuration Phase Count mismatch!");
        SyrMetronomeConfig_Destroy(config);
        return SYR_RESULT_FAILED;
    }

    if (config->phaseCount == 0)
    {
        SYR_ERROR("Metronome Configure Phase Count Can't be 0!");
        SyrMetronomeConfig_Destroy(config);
        return SYR_RESULT_FAILED;
    }

    SyrMetronome_Clear(metronome);

    for (uint32_t i = 0; i < config->phaseCount; i++)
    {
        SyrMetronomePhase* phase = SyrMetronome_AddPhase(metronome,
            &config->phaseConfigs[i]);

        if (phase == NULL)
        {
            SyrMetronomeConfig_Destroy(config);
            return SYR_RESULT_FAILED;
        }
    }

    SyrMetronomeConfig_Destroy(config);
    return SYR_RESULT_SUCCESS;
}

static inline SyrResult SyrMetronome_ValidatePhase(const SyrMetronome* metronome,
    const uint32_t phaseIndex)
{
    if (phaseIndex >= SyrList_Count(metronome->phases))
    {
        SYR_ERROR("Phase Index is greater than Phases in Metronome!");
        return SYR_RESULT_RUNTIME_ERROR;
    }

    const SyrMetronomePhase* phase = &metronome->phases[phaseIndex];

    if (phase->instrumentCount != SyrList_Count(phase->instrumentBindings))
    {
        SYR_ERROR("Invalid Metronome Phase: Required Count does not equal Bindings Count!");
        return SYR_RESULT_RUNTIME_ERROR;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrMetronome_RecordPhaseBarriers(SyrMetronome* metronome,
    SyrCommandBuffer* commandBuffer,
    const uint32_t phaseIndex,
    const SyrAudioBuffer* masterBuffer,
    SyrList(SyrInstrument*) instruments)
{
    if (SyrMetronome_ValidatePhase(metronome, phaseIndex) != SYR_RESULT_SUCCESS)
    {
        return SYR_RESULT_RUNTIME_ERROR;
    }

    const SyrMetronomePhase* phase = &metronome->phases[phaseIndex];
    SyrBarrierBatch* barrierBatch = NULL;
    const SyrResourceAction previousAction = phaseIndex != 0 ? SYR_RESOURCE_ACTION_BUFFER_READ_WRITE : SYR_RESOURCE_ACTION_UNDEFINED;
    const SyrResourceAction targetAction = SYR_RESOURCE_ACTION_BUFFER_READ_WRITE;
    const uint32_t barrierCount = phase->instrumentCount * SYR_INSTRUMENT_SSBO_COUNT
        + (phase->requiresMaster ? SYR_MASTER_SSBO_COUNT : 0);

    if (barrierCount == 0)
        return SYR_RESULT_SUCCESS;

    if (SyrBarrierBatch_Initialize(previousAction,
            targetAction,
            barrierCount,
            &barrierBatch)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to Initialize Barrier Batch with Count: %u!", barrierCount);
        SyrBarrierBatch_Destroy(barrierBatch);
        barrierBatch = NULL;
        return SYR_RESULT_RUNTIME_ERROR;
    }

    for (uint32_t i = 0; i < phase->instrumentCount; i++)
    {
        const SyrInstrumentBinding* binding = &phase->instrumentBindings[i];

        if (binding->instrumentIndex >= SyrList_Count(instruments))
        {
            SYR_ERROR("Phase Instrument Index out of range!");
            SyrBarrierBatch_Destroy(barrierBatch);
            barrierBatch = NULL;
            return SYR_RESULT_RUNTIME_ERROR;
        }

        const SyrAudioBuffer* audioBuffer = SyrInstrument_GetAudioBuffer(instruments[binding->instrumentIndex]);

        SyrBarrierBatch_AttachBuffer(barrierBatch,
            audioBuffer->timeAllocation,
            i * SYR_INSTRUMENT_SSBO_COUNT);

        SyrBarrierBatch_AttachBuffer(barrierBatch,
            audioBuffer->frequencyAllocation,
            i * SYR_INSTRUMENT_SSBO_COUNT + 1);
    }

    if (phase->requiresMaster)
    {
        SyrBarrierBatch_AttachBuffer(barrierBatch,
            masterBuffer->timeAllocation,
            phase->instrumentCount * SYR_INSTRUMENT_SSBO_COUNT);

        SyrBarrierBatch_AttachBuffer(barrierBatch,
            masterBuffer->frequencyAllocation,
            phase->instrumentCount * SYR_INSTRUMENT_SSBO_COUNT + 1);
    }

    SyrCommandBuffer_RecordBarrierBatch(commandBuffer, barrierBatch);

    SyrBarrierBatch_Destroy(barrierBatch);

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrMetronome_BindPhaseBuffers(SyrMetronome* metronome,
    SyrCommandBuffer* commandBuffer,
    SyrChord* chord,
    const uint32_t phaseIndex,
    const SyrAudioBuffer* masterBuffer,
    SyrList(SyrInstrument*) instruments)
{
    if (SyrMetronome_ValidatePhase(metronome, phaseIndex) != SYR_RESULT_SUCCESS)
    {
        return SYR_RESULT_RUNTIME_ERROR;
    }

    const SyrMetronomePhase* phase = &metronome->phases[phaseIndex];

    if (SyrChord_GetInstrumentCount(chord) != phase->instrumentCount)
    {
        SYR_ERROR("Chord (name: %s) and Phase Instrument Count Mismatch (chord: %u, phase: %u)!",
            SyrChord_GetName(chord),
            SyrChord_GetInstrumentCount(chord),
            phase->instrumentCount);

        return SYR_RESULT_RUNTIME_ERROR;
    }

    if (phase->requiresMaster)
    {
        if (SyrChord_WriteMaster(chord, masterBuffer) != SYR_RESULT_SUCCESS)
        {
            SYR_ERROR("Failed to Write Master Buffer to Chord: %s", SyrChord_GetName(chord));
            return SYR_RESULT_RUNTIME_ERROR;
        }
    }

    for (uint32_t i = 0; i < phase->instrumentCount; i++)
    {
        const SyrInstrumentBinding* binding = &phase->instrumentBindings[i];

        if (binding->instrumentIndex >= SyrList_Count(instruments))
        {
            SYR_ERROR("Phase Instrument Index out of range!");
            return SYR_RESULT_RUNTIME_ERROR;
        }

        if (SyrChord_WriteInstrument(chord,
                instruments[binding->instrumentIndex],
                (uint32_t)binding->instrumentSlot)
            != SYR_RESULT_SUCCESS)
        {
            SYR_ERROR("Failed to Write Instrument Buffer to Chord: %s, at Instrument Slot %u",
                SyrChord_GetName(chord),
                (uint32_t)binding->instrumentSlot);

            return SYR_RESULT_RUNTIME_ERROR;
        }
    }

    if (SyrChord_WriteInstrumentData(chord) != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to Write Instrument Data to Chord %s",
            SyrChord_GetName(chord));

        return SYR_RESULT_RUNTIME_ERROR;
    }

    if (SyrChord_WriteFFTConstants(chord, commandBuffer) != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to Push FFT Constants to Chord %s",
            SyrChord_GetName(chord));

        return SYR_RESULT_RUNTIME_ERROR;
    }

    return SYR_RESULT_SUCCESS;
}

uint32_t SyrMetronome_GetDispatchSamples(SyrMetronome* metronome,
    const uint32_t phaseIndex)
{
    if (phaseIndex >= (uint32_t)SyrList_Count(metronome->phases))
        return 0;

    return metronome->phases[phaseIndex].dispatchSamples;
}

void SyrMetronome_Clear(SyrMetronome* metronome)
{
    for (size_t i = 0; i < SyrList_Count(metronome->phases); i++)
    {
        SyrList_Free(metronome->phases[i].instrumentBindings);
    }

    SyrList_Clear(metronome->phases);
}

void SyrMetronome_Destroy(SyrMetronome* metronome)
{
    if (metronome == NULL)
        return;

    if (metronome->phases != NULL)
    {
        SyrMetronome_Clear(metronome);
        SyrList_Free(metronome->phases);
    }

    SYR_FREE(metronome);
}

void SyrMetronomeConfig_Destroy(SyrMetronomeConfig* metronomeConfig)
{
    if (metronomeConfig == NULL)
        return;

    for (size_t i = 0; i < SyrList_Count(metronomeConfig->phaseConfigs); i++)
    {
        SyrList_Free(metronomeConfig->phaseConfigs[i].bindings);
    }

    SyrList_Free(metronomeConfig->phaseConfigs);
    SYR_FREE(metronomeConfig);
}
