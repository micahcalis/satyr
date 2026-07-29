#include "Core/SatyrCore.h"
#include "Core/Application.h"

static const SyrConfig SYR_MAIN_CONFIG = {
    .initializeOnStartup = true};

int main()
{
    SyrApplication* app = NULL;

    SyrResult result = SyrApplication_Initialize(&SYR_MAIN_CONFIG, &app);

    if (result == SYR_RESULT_SUCCES)
    {
        SYR_LOG("Satyr initialized succesfully");
    } else if (result == SYR_RESULT_VULKAN_FAILED)
    {
        SYR_ERROR("Satyr Vulkan failed!");
    }

    SyrApplication_Terminate(app);
}
