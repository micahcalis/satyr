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
}

// SYR_TG_SIZE_L
// void SyrKernel_0(uint3 id : SV_DispatchThreadID)
// {
//     // // _ExampleTime[0] = 2.0f;
//     // // _ExampleFrequency[0] = Satyr.Complex(0, 0);

//     // float sampleRate = 48000.0f; 
//     // float t = float(id.x) / sampleRate;
//     // float interpolator = (float)id.x / (1024.0f * 100.0f);
//     // float ease = cos(interpolator * 1200.0f) * 0.5f + 0.5f;
//     // float ease2 = sin(interpolator * 1200.0f) * 0.5f + 0.5f;
//     // float frequency = lerp(0.5f, 4.0f, ease * ease2);
//     // float amplitude = 0.5f;  
//     // float twoPi = 6.28318530718f;

//     // float sampleValue = amplitude * sin(twoPi * frequency * t);
//     // _MasterTime[id.x] = _ExampleTime[id.x];


// }

SYR_TG_SIZE_L
void SyrKernel_0(uint3 localID : SV_GroupThreadID, uint groupID : SV_GroupID, uint3 id : SV_DispatchThreadID)
{
    Satyr.FFTHandle fftHandle = Satyr.FFTHandle();

    Satyr.FFTThreadContainer threadContainer = Satyr.FFTThreadContainer(localID.x,
        groupID,
        _STFTConstants.threadGroupSize,
        _STFTConstants.fftSize);

    Satyr.FFTComputer<Satyr.FFTHandle> fftComputer = Satyr.FFTComputer<Satyr.FFTHandle>(
        fftHandle, _STFTConstants, Satyr.FFTDirection::Forward, threadContainer);

    fftComputer.Compute();
}

SYR_TG_SIZE_L
void SyrKernel_1(uint3 id : SV_DispatchThreadID, uint groupID : SV_GroupID)
{
    Satyr.FFTHandle fftHandle = Satyr.FFTHandle();
    Satyr.Complex freq = fftHandle.ReadFrequency(id.x);

    uint fftSize = _STFTConstants.fftSize;
    uint localBin = id.x % fftSize;
    uint distanceFromDC = min(localBin, fftSize - localBin);

    // 1. Get the raw volume (Magnitude) of this specific frequency
    float mag = sqrt((freq.real * freq.real) + (freq.imaginary * freq.imaginary));

    // 2. THE SPECTRAL GATE (Cleans up the pauses)
    // Audio magnitudes in an FFT scale with the fftSize. 
    // You may need to tune this threshold (try 1.0f, 5.0f, or 20.0f) based on how loud the MP3 is.
    float gateThreshold = 1.5f; 
    if (mag < gateThreshold)
    {
        mag = 0.0f;
    }

    // 3. THE ROBOT VOCODER
    // By forcing all the energy into the real part and zeroing the imaginary part, 
    // we destroy the human vocal chords and replace it with a metallic buzz.
    // (Because 0.0f is its own negative, this perfectly respects conjugate symmetry!)
    freq.real = mag;
    freq.imaginary = 0.0f;

    // 4. THE TELEPHONE EQ
    // Mute everything below bin 15 (~700 Hz) to remove the bass/chest resonance.
    if (distanceFromDC < 20)
    {
        freq.real = 0.0f;
        freq.imaginary = 0.0f;
    }

    freq.real *= 10.0f;
    freq.imaginary *= 5.0f;

    fftHandle.WriteFrequency(id.x, freq);
}

SYR_TG_SIZE_L
void SyrKernel_2(uint3 localID : SV_GroupThreadID, uint groupID : SV_GroupID)
{
    Satyr.FFTHandle fftHandle = Satyr.FFTHandle();

    Satyr.FFTThreadContainer threadContainer = Satyr.FFTThreadContainer(localID.x,
        groupID,
        _STFTConstants.threadGroupSize,
        _STFTConstants.fftSize);

    Satyr.FFTComputer<Satyr.FFTHandle> fftComputer = Satyr.FFTComputer<Satyr.FFTHandle>(
        fftHandle, _STFTConstants, Satyr.FFTDirection::Inverse, threadContainer);

    fftComputer.Compute();
}

SYR_TG_SIZE_L
void SyrKernel_3(uint3 id : SV_DispatchThreadID, uint groupID : SV_GroupID)
{
    Satyr.FFTHandle fftHandle = Satyr.FFTHandle();
    _MasterTime[id.x] = fftHandle.ReadTime(id.x);
}