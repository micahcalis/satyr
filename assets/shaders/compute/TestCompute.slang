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
void SyrKernel_2(uint3 localID : SV_GroupThreadID, uint groupID : SV_GroupID)
{
    Satyr::ExampleKernelDSPEngine<Satyr::FFTKernelPhase::InverseFFT>(localID.x, groupID);
}

SYR_TG_SIZE_L
void SyrKernel_3(uint3 localID : SV_GroupThreadID, uint groupID : SV_GroupID)
{
    Satyr::ExampleKernelDSPEngine<Satyr::FFTKernelPhase::OverlapAddTime>(localID.x, groupID);
}

SYR_TG_SIZE_L
void SyrKernel_4(uint3 id : SV_DispatchThreadID, uint groupID : SV_GroupID)
{
    Satyr::FFTHandle fftHandle = Satyr::FFTHandle();
    _MasterTime[id.x] = fftHandle.ReadTime(id.x);
}

// SYR_TG_SIZE_L
// void SyrKernel_0(uint3 localID : SV_GroupThreadID, uint groupID : SV_GroupID)
// {
//     Satyr.FFTHandle fftHandle = Satyr.FFTHandle();
//     Satyr.InstrumentData instrumentData = _InstrumentData[(uint)Satyr.InstrumentDataSlot::Instrument0];

//     Satyr.FFTThreadContainer threadContainer = Satyr.FFTThreadContainer(localID.x,
//         groupID,
//         _STFTConstants.threadGroupSize,
//         _STFTConstants.fftSize,
//         (uint)instrumentData.totalSamples,
//         instrumentData.paddingSamples);

//     Satyr.WOLAMixer<Satyr.FFTHandle> wolaMixer = Satyr.WOLAMixer<Satyr.FFTHandle>(
//         fftHandle, _STFTConstants, Satyr.WOLADirection::Expand, threadContainer);

//     wolaMixer.Mix();
// }

// SYR_TG_SIZE_L
// void SyrKernel_1(uint3 localID : SV_GroupThreadID, uint groupID : SV_GroupID)
// {
//     Satyr.FFTHandle fftHandle = Satyr.FFTHandle();
//     Satyr.InstrumentData instrumentData = _InstrumentData[(uint)Satyr.InstrumentDataSlot::Instrument0];

//     Satyr.FFTThreadContainer threadContainer = Satyr.FFTThreadContainer(localID.x,
//         groupID,
//         _STFTConstants.threadGroupSize,
//         _STFTConstants.fftSize,
//         (uint)instrumentData.totalSamples,
//         instrumentData.paddingSamples);

//     Satyr.FFTComputer<Satyr.FFTHandle> fftComputer = Satyr.FFTComputer<Satyr.FFTHandle>(
//         fftHandle, _STFTConstants, Satyr.FFTDirection::Forward, threadContainer);

//     fftComputer.Compute();
// }

// SYR_TG_SIZE_L
// void SyrKernel_2(uint3 id : SV_DispatchThreadID, uint groupID : SV_GroupID)
// {
//     Satyr.FFTHandle fftHandle = Satyr.FFTHandle();
//     Satyr.Complex freq = fftHandle.ReadFrequency(id.x);

//     uint fftSize = _STFTConstants.fftSize;
//     uint localBin = id.x % fftSize;
//     uint distanceFromDC = min(localBin, fftSize - localBin);
//     float mag = sqrt((freq.real * freq.real) + (freq.imaginary * freq.imaginary));

//     float gateThreshold = 1.5f;
//     if (mag < gateThreshold)
//     {
//         mag = 0.0f;
//     }

//     freq.real = mag;
//     freq.imaginary = 0.0f;

//     if (distanceFromDC < 20)
//     {
//         freq.real = 0.0f;
//         freq.imaginary = 0.0f;
//     }

//     freq.real *= 10.0f;
//     freq.imaginary *= 5.0f;

//     // temporarily turned off to test functionality
//     //fftHandle.WriteFrequency(id.x, freq);
// }

// SYR_TG_SIZE_L
// void SyrKernel_3(uint3 localID : SV_GroupThreadID, uint groupID : SV_GroupID)
// {
//     Satyr.FFTHandle fftHandle = Satyr.FFTHandle();
//     Satyr.InstrumentData instrumentData = _InstrumentData[(uint)Satyr.InstrumentDataSlot::Instrument0];

//     Satyr.FFTThreadContainer threadContainer = Satyr.FFTThreadContainer(localID.x,
//         groupID,
//         _STFTConstants.threadGroupSize,
//         _STFTConstants.fftSize,
//         (uint)instrumentData.totalSamples,
//         instrumentData.paddingSamples);

//     Satyr.FFTComputer<Satyr.FFTHandle> fftComputer = Satyr.FFTComputer<Satyr.FFTHandle>(
//         fftHandle, _STFTConstants, Satyr.FFTDirection::Inverse, threadContainer);

//     fftComputer.Compute();
// }

// SYR_TG_SIZE_L
// void SyrKernel_4(uint3 localID : SV_GroupThreadID, uint groupID : SV_GroupID)
// {
//     Satyr.FFTHandle fftHandle = Satyr.FFTHandle();
//     Satyr.InstrumentData instrumentData = _InstrumentData[(uint)Satyr.InstrumentDataSlot::Instrument0];

//     Satyr.FFTThreadContainer threadContainer = Satyr.FFTThreadContainer(localID.x,
//         groupID,
//         _STFTConstants.threadGroupSize,
//         _STFTConstants.fftSize,
//         (uint)instrumentData.totalSamples,
//         instrumentData.paddingSamples);

//     Satyr.WOLAMixer<Satyr.FFTHandle> wolaMixer = Satyr.WOLAMixer<Satyr.FFTHandle>(
//         fftHandle, _STFTConstants, Satyr.WOLADirection::Add, threadContainer);

//     wolaMixer.Mix();
// }

// SYR_TG_SIZE_L
// void SyrKernel_5(uint3 id : SV_DispatchThreadID, uint groupID : SV_GroupID)
// {
//     Satyr.FFTHandle fftHandle = Satyr.FFTHandle();
//     _MasterTime[id.x] = fftHandle.ReadTime(id.x);
// }
