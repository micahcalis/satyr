#include "Application.h"
#include "SatyrCore.h"

typedef struct SyrApplication
{
    bool isRunning;
} SyrApplication;

SyrResult SyrApplication_Initialize(const SyrConfig* config, SyrApplication** application)
{
    *application = malloc(sizeof(SyrApplication));
    (*application)->isRunning = true;

    return config->initializeOnStartup ? SYR_RESULT_SUCCES : SYR_RESULT_VULKAN_FAILED;
}

void SyrApplication_Terminate(SyrApplication* application)
{
    if (application != NULL)
    {
        SYR_LOG("Terminate Satyr");
        free(application);
    }
}
