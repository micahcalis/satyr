#pragma once

#include "Core/SatyrCore.h"

typedef struct SyrDevice SyrDevice;

SyrResult SyrDevice_Initialize(const SyrConfig* config, SyrDevice** device);
void SyrDevice_Destroy(SyrDevice* device);
