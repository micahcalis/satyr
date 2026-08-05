#include "Core/SyrinxMacros.slang"

SYR_SHADER_PARAMS
{
    float _Test;
}

SYR_RW_BUFFER(_TestBuffer, float4, 1)

SYR_COMPUTE_KERNEL(0, SYR_TG_SIZE_L)
{
    uint testId = id.x;
    _TestBuffer[testId] = float4(1, 1, 0, 0);
}