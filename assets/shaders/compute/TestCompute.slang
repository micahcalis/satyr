#include "SatyrMacros.slang"

SYR_NOTES_DATA
{
    float _Test;
}

SYR_INSTRUMENT_DATA
SYR_MASTER_BUFFER
SYR_INSTRUMENT_BUFFER(_Example, 0)
SYR_FFT_CONSTANTS

SYR_TG_SIZE_L
void SyrKernel_0(uint3 id : SV_DispatchThreadID)
{
    _ExampleTime[0] = 2.0f;
    _ExampleFrequency[0] = Satyr.Complex(0, 0);

    float sampleRate = 48000.0f; 
    float t = float(id.x) / sampleRate;
    float interpolator = (float)id.x / (1024.0f * 100.0f);
    float ease = cos(interpolator * 1200.0f) * 0.5f + 0.5f;
    float ease2 = sin(interpolator * 1200.0f) * 0.5f + 0.5f;
    float frequency = lerp(0.5f, 4.0f, ease * ease2);
    float amplitude = 0.5f;  
    float twoPi = 6.28318530718f;

    float sampleValue = amplitude * sin(twoPi * frequency * t);
    _MasterTime[id.x] = sampleValue;
}