#pragma once

#include "SatyrCore.h"

typedef struct SyrApplication SyrApplication;

SyrResult SyrApplication_Bootup(SyrApplication* application, const SyrConfig* config);
SyrResult SyrApplication_Initialize(const SyrConfig* config, SyrApplication** application);
void SyrApplication_Run(SyrApplication* application);
void SyrApplication_Cleanup(SyrApplication* application);
void SyrApplication_Terminate(SyrApplication* application);

