#include "Application.h"
#include "SatyrCore.h"
#include "Syrinx.h"

typedef struct SyrApplication
{
    bool isRunning;
    SyrSyrinx* syrinx;

} SyrApplication;

SyrResult SyrApplication_Bootup(SyrApplication* application, const SyrConfig* config)
{
    SyrResult syrinxResult = SyrSyrinx_InitializeVulkan(application->syrinx, config);

    return syrinxResult;
}

SyrResult SyrApplication_Initialize(const SyrConfig* config, SyrApplication** application)
{
    *application = SYR_NEW((*application));
    (*application)->isRunning = false;

    (*application)->syrinx = SyrSyrinx_Create(config);

    if (config->bootupOnStartup)
    {
        return SyrApplication_Bootup((*application), config);
    } else
    {
        return SYR_RESULT_WAITING;
    }
}

void SyrApplication_Run(SyrApplication* application)
{
    if (application == NULL)
    {
        SYR_ERROR("Can't Run uninitialized appplication");
    }

    application->isRunning = true;

    SYR_LOG("Satyr is Running...");
    SYR_LOG("Press Enter to Quit");

    getchar();

    application->isRunning = false;
}

void SyrApplication_Cleanup(SyrApplication* application)
{
    SyrSyrinx_Destroy(application->syrinx);
}

void SyrApplication_Terminate(SyrApplication* application)
{
    SyrApplication_Cleanup(application);

    if (application != NULL)
    {
        SYR_LOG("Terminate Satyr");
        free(application);
    }
}
