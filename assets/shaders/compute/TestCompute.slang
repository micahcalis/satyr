#include "Core/SatyrMacros.slang"

SYR_SHADER_PARAMS
{
    float _Test;
}

SYR_MASTER_BUFFER
SYR_INSTRUMENT_BUFFER(_Example, 0)

SYR_TG_SIZE_L
void SyrKernel_0(uint3 id : SV_DispatchThreadID)
{
    _ExampleTime[0] = 2.0f;
    _ExampleFrequency[0] = (Satyr.Complex)0;
    _MasterTime[0] = 0.234f;
}