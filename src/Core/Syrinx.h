#pragma once

#include "SatyrCore.h"

typedef struct SyrSyrinx SyrSyrinx;

SyrSyrinx* SyrSyrinx_Create(const SyrConfig* config);
SyrResult SyrSyrinx_InitializeVulkan(SyrSyrinx* syrinx, const SyrConfig* config);
void SyrSyrinx_Destroy(SyrSyrinx* syrinx);
