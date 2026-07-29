#include "VulkInstance.h"
#include "Core/SatyrCore.h"

typedef struct SyrVulkInstance
{
} SyrVulkInstance;

SyrResult SyrVulkInstance_Initialize(const SyrConfig* config, SyrVulkInstance** vulkInstance)
{
    vulkInstance = SYR_NEW((*vulkInstance));

    return SYR_RESULT_SUCCESS;
}

void SyrVulkInstance_Destroy(SyrVulkInstance* vulkInstance)
{
    free(vulkInstance);
}
