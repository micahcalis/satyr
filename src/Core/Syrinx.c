#include "Syrinx.h"
#include "SatyrCore.h"

typedef struct SyrSyrinx
{
    int test;
} SyrSyrinx;

SyrSyrinx* SyrSyrinx_Create(const SyrConfig* config)
{
    SyrSyrinx* syrinx = malloc(sizeof(SyrSyrinx));
    syrinx->test = 10;

    return syrinx;
}

SyrResult SyrSyrinx_InitializeVulkan(SyrSyrinx* syrinx, const SyrConfig* config)
{
    return SYR_RESULT_SUCCESS;
}
