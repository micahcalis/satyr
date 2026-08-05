#include "Satyr.h"

static const SyrConfig SYR_MAIN_CONFIG = {
    .bootupOnStartup = true,
    .pipelineCachePath = "C:/Users/micah/Desktop/Hobby/satyr/bin/pipeline_cache.bin"};

int main()
{
    SyrApplication* app = NULL;

    SyrResult result = SyrApplication_Initialize(&SYR_MAIN_CONFIG, &app);

    if (result == SYR_RESULT_SUCCESS)
    {
        SYR_LOG("Satyr initialized succesfully");
    } else if (result == SYR_RESULT_VULKAN_FAILED)
    {
        SYR_ERROR("Satyr Vulkan failed!");
    }

    SyrApplication_Run(app);
    SyrApplication_Terminate(app);
}
