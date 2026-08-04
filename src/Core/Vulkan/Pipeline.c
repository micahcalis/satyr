#include "Pipeline.h"

typedef struct SyrPipelineCache
{
    VkPipelineCache cacheHandle;
    VkDevice device;
    char* cachePath;
} SyrPipelineCache;

typedef struct SyrPipeline SyrPipeline;

static SyrResult SyrPipelineCache_TryLoadCacheFile(const char* cachePath,
    size_t* fileSize,
    char** fileBuffer)
{
    FILE* file;
    errno_t err = fopen_s(&file, cachePath, "rb");

    if (err != 0 || file == NULL)
    {
        return SYR_RESULT_FAILED;
    }

    fseek(file, 0, SEEK_END);
    long rawSize = ftell(file);
    *fileSize = (size_t)rawSize;

    rewind(file);

    *fileBuffer = (char*)malloc(*fileSize);

    if (*fileBuffer == NULL)
    {
        SYR_ERROR("Failed allocating file buffer!");
        fclose(file);
        return SYR_RESULT_RUNTIME_ERROR;
    }

    fread(*fileBuffer, 1, *fileSize, file);
    fclose(file);

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrPipelineCache_Initialize(const SyrConfig* config,
    SyrDevice* device,
    SyrPipelineCache** pipelineCache)
{
    *pipelineCache = SYR_NEW(*pipelineCache);
    (*pipelineCache)->cacheHandle = VK_NULL_HANDLE;
    (*pipelineCache)->device = SyrDevice_GetLogicalDeviceHandle(device);
    (*pipelineCache)->cachePath = _strdup(config->pipelineCachePath);

    VkPipelineCacheCreateInfo cacheInfo = {0};
    cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    size_t fileSize = 0;
    char* fileBuffer = NULL;

    SyrResult loadResult = SyrPipelineCache_TryLoadCacheFile(config->pipelineCachePath,
        &fileSize,
        &fileBuffer);

    if (loadResult == SYR_RESULT_SUCCESS)
    {
        cacheInfo.initialDataSize = fileSize;
        cacheInfo.pInitialData = fileBuffer;
    } else if (loadResult == SYR_RESULT_RUNTIME_ERROR)
    {
        SyrPipelineCache_Destroy(*pipelineCache);
        return SYR_RESULT_RUNTIME_ERROR;
    }

    if (vkCreatePipelineCache((*pipelineCache)->device,
            &cacheInfo,
            NULL,
            &(*pipelineCache)->cacheHandle)
        != VK_SUCCESS)
    {
        SYR_ERROR("Failed to create pipeline cache");
        SyrPipelineCache_Destroy(*pipelineCache);
        free(fileBuffer);
        return SYR_RESULT_VULKAN_FAILED;
    }

    free(fileBuffer);
    return SYR_RESULT_SUCCESS;
}

static SyrResult SyrPipelineCache_SaveCache(SyrPipelineCache* pipelineCache)
{
#ifdef SYR_DISABLE_VULKAN_PIPELINE_CACHE
    size_t cacheSize = 0;

    if (vkGetPipelineCacheData(pipelineCache->device,
            pipelineCache->cacheHandle,
            &cacheSize,
            NULL)
        != VK_SUCCESS)
    {
        SYR_ERROR("Failed to get Pipeline Cache Data!");
        return SYR_RESULT_VULKAN_FAILED;
    }

    if (cacheSize > 0)
    {
        void* cacheData = malloc(cacheSize);

        if (vkGetPipelineCacheData(pipelineCache->device,
                pipelineCache->cacheHandle,
                &cacheSize,
                cacheData)
            != VK_SUCCESS)
        {
            SYR_ERROR("Failed to get Pipeline Cache Data!");
            free(cacheData);
            return SYR_RESULT_VULKAN_FAILED;
        }

        FILE* file;
        if (fopen_s(&file, pipelineCache->cachePath, "wb") == 0)
        {
            fwrite(cacheData, 1, cacheSize, file);
            fclose(file);
        }
        free(cacheData);
    }
    return SYR_RESULT_SUCCESS;
#endif
    return SYR_RESULT_FAILED;
}

void SyrPipelineCache_Destroy(SyrPipelineCache* pipelineCache)
{
    if (pipelineCache == NULL)
        return;

    if (pipelineCache->cacheHandle != VK_NULL_HANDLE)
    {
        if (SyrPipelineCache_SaveCache(pipelineCache) == SYR_RESULT_VULKAN_FAILED)
        {
            SYR_ERROR("Failed to save Pipeline Cache to: %s", pipelineCache->cachePath);
        }

        vkDestroyPipelineCache(pipelineCache->device,
            pipelineCache->cacheHandle,
            NULL);
    }

    if (pipelineCache->cachePath != NULL)
    {
        free(pipelineCache->cachePath);
    }

    free(pipelineCache);
}
