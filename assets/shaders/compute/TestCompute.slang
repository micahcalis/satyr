#include "SatyrMacros.slang"

SYR_NOTES_DATA
{
    float _Test;
}

SYR_INSTRUMENT_DATA
SYR_MASTER_BUFFER
SYR_INSTRUMENT_BUFFER(_Example, 0)
SYR_FFT_CONSTANTS
SYR_FFT_SHARED(Satyr.FFTSize::L);

namespace Satyr
{
    public struct FFTHandle : Satyr.IFFTHandle
    {
        public Complex ReadShared(uint index)
        {
            return _STFTShared[index];
        }

        public void WriteShared(uint index, Complex value)
        {
            _STFTShared[index] = value;
        }

        public float ReadTime(uint index)
        {
            return _ExampleTime[index];
        }

        public void WriteTime(uint index, float value)
        {
            _ExampleTime[index] = value;
        }

        public Complex ReadFrequency(uint index)
        {
            return _ExampleFrequency[index];
        }

        public void WriteFrequency(uint index, Complex value)
        {
            _ExampleFrequency[index] = value;
        }
    }

    public void ExampleKernelDSPEngine<let kernelPhase : FFTKernelPhase>(uint localID,
        uint groupID)
    {
        FFTHandle handle = FFTHandle();
        InstrumentData instrumentData = _InstrumentData[(uint)InstrumentDataSlot::Instrument0];

        DSPEngineData engineData = DSPEngineData(localID,
            groupID,
            _STFTConstants,
            instrumentData);

        DSPEngine<FFTHandle, kernelPhase> engine = DSPEngine<FFTHandle, kernelPhase>(handle,
            engineData);

        engine.ExecutePhase();
    }

    public struct PVHandle : IPVHandle
    {
        InstrumentData instrumentData;

        public __init()
        {
            instrumentData = _InstrumentData[(uint)InstrumentDataSlot::Instrument0];
        }

        public InstrumentData GetInstrumentData()
        {
            return instrumentData;
        }

        public Complex ReadFrequency(uint index)
        {
            return _ExampleFrequency[index];
        }

        public void WriteFrequency(uint index, Complex value)
        {
            _ExampleFrequency[index] = value;
        }
    }

    public static void ApplyRobotizationEffect(uint threadIndex)
    {
        PVHandle handle = PVHandle();
        RobotizationEffect effect = RobotizationEffect();

        PhaseVocoder<PVHandle, RobotizationEffect> phaseVocoder = PhaseVocoder<PVHandle, RobotizationEffect>(
            handle,
            effect,
            threadIndex,
            0);

        phaseVocoder.ApplyEffect();
    }
}

SYR_TG_SIZE_L
void SyrKernel_0(uint3 localID : SV_GroupThreadID, uint groupID : SV_GroupID)
{
    Satyr::ExampleKernelDSPEngine<Satyr::FFTKernelPhase::ExpandTime>(localID.x, groupID);
}

SYR_TG_SIZE_L
void SyrKernel_1(uint3 localID : SV_GroupThreadID, uint groupID : SV_GroupID)
{
    Satyr::ExampleKernelDSPEngine<Satyr::FFTKernelPhase::ForwardFFT>(localID.x, groupID);
}

SYR_TG_SIZE_L
void SyrKernel_2(uint3 threadID : SV_DispatchThreadID)
{
    Satyr::ApplyRobotizationEffect(threadID.x);
}

SYR_TG_SIZE_L
void SyrKernel_3(uint3 localID : SV_GroupThreadID, uint groupID : SV_GroupID)
{
    Satyr::ExampleKernelDSPEngine<Satyr::FFTKernelPhase::InverseFFT>(localID.x, groupID);
}

SYR_TG_SIZE_L
void SyrKernel_4(uint3 localID : SV_GroupThreadID, uint groupID : SV_GroupID)
{
    Satyr::ExampleKernelDSPEngine<Satyr::FFTKernelPhase::OverlapAddTime>(localID.x, groupID);
}

SYR_TG_SIZE_L
void SyrKernel_5(uint3 threadID : SV_DispatchThreadID, uint groupID : SV_GroupID)
{
    Satyr::FFTHandle fftHandle = Satyr::FFTHandle();
    _MasterTime[threadID.x] = fftHandle.ReadTime(threadID.x);
}
