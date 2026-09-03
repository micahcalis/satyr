#include "Pipeline.h"
#include "Core/Vulkan/ShaderModule.h"

typedef struct SyrPipelineCache
{
    VkPipelineCache cacheHandle;
    VkDevice device;
    char* cachePath;
} SyrPipelineCache;

typedef struct SyrPipeline
{
    VkPipeline pipelineHandle;
    VkPipelineLayout layoutHandle;
    VkDevice device;
    uint32_t kernelIndex;
} SyrPipeline;

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
        SYR_FREE(fileBuffer);
        return SYR_RESULT_VULKAN_FAILED;
    }

    SYR_FREE(fileBuffer);
    return SYR_RESULT_SUCCESS;
}

static SyrResult SyrPipelineCache_SaveCache(SyrPipelineCache* pipelineCache)
{
#ifdef SYR_ENABLE_VULKAN_PIPELINE_CACHE
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
            SYR_FREE(cacheData);
            return SYR_RESULT_VULKAN_FAILED;
        }

        FILE* file;
        if (fopen_s(&file, pipelineCache->cachePath, "wb") == 0)
        {
            fwrite(cacheData, 1, cacheSize, file);
            fclose(file);
        }
        SYR_FREE(cacheData);
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
        SYR_FREE(pipelineCache->cachePath);
    }

    SYR_FREE(pipelineCache);
}

static void SyrPipeline_GetEntryPointName(const uint32_t index, size_t maxLength, char* name)
{
    snprintf(name, maxLength, "SyrKernel_%u", index);
}

SyrResult SyrPipeline_CreateLayout(SyrPipeline* pipeline,
    VkDescriptorSetLayout layout,
    const char* shaderPath,
    const size_t pushConstantsSize)
{
    VkPushConstantRange pushConstantRange = {0};
    pushConstantRange.size = pushConstantsSize;
    pushConstantRange.offset = 0;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkPipelineLayoutCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = 1;
    createInfo.pSetLayouts = &layout;
    createInfo.pushConstantRangeCount = 1;
    createInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(pipeline->device,
            &createInfo,
            NULL,
            &pipeline->layoutHandle)
        != VK_SUCCESS)
    {
        SYR_ERROR("Failed to create Pipeline Layout for index: %u, path: %s", pipeline->kernelIndex, shaderPath);
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrPipeline_CreatePipeline(SyrPipeline* pipeline,
    SyrPipelineCache* pipelineCache,
    VkShaderModule shaderModule,
    const char* entryPointName,
    const char* shaderPath)
{
    VkPipelineShaderStageCreateInfo stageCreateInfo = {0};
    stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageCreateInfo.module = shaderModule;
    stageCreateInfo.pName = entryPointName;

    VkComputePipelineCreateInfo pipelineCreateInfo = {0};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = stageCreateInfo;
    pipelineCreateInfo.layout = pipeline->layoutHandle;

    VkPipelineCache cacheHandle = VK_NULL_HANDLE;

#ifdef SYR_ENABLE_VULKAN_PIPELINE_CACHE
    cacheHandle = pipelineCache->cacheHandle;
#endif

    if (vkCreateComputePipelines(pipeline->device,
            cacheHandle,
            1,
            &pipelineCreateInfo,
            NULL,
            &pipeline->pipelineHandle)
        != VK_SUCCESS)
    {
        SYR_ERROR("Failed to create Pipeline for index: %u, path: %s", pipeline->kernelIndex, shaderPath);
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrPipeline_Initialize(const char* shaderPath,
    const uint32_t kernelIndex,
    const size_t pushConstantsSize,
    VkDescriptorSetLayout setLayout,
    SyrDevice* device,
    SyrPipelineCache* pipelineCache,
    SyrPipeline** pipeline)
{
    *pipeline = SYR_NEW(*pipeline);
    (*pipeline)->device = SyrDevice_GetLogicalDeviceHandle(device);
    (*pipeline)->kernelIndex = kernelIndex;

    char entryPointName[64];
    SyrPipeline_GetEntryPointName(kernelIndex, sizeof(entryPointName), entryPointName);

    SyrShaderModule* shaderModule = NULL;

    if (SyrShaderModule_Initialize(shaderPath,
            (*pipeline)->device,
            &shaderModule)
        != SYR_RESULT_SUCCESS)
    {
        SyrPipeline_Destroy(*pipeline);
        *pipeline = NULL;
        return SYR_RESULT_VULKAN_FAILED;
    }

    if (SyrPipeline_CreateLayout(*pipeline,
            setLayout,
            shaderPath,
            pushConstantsSize)
        != SYR_RESULT_SUCCESS)
    {
        SyrPipeline_Destroy(*pipeline);
        SyrShaderModule_Destroy(shaderModule);
        *pipeline = NULL;
        return SYR_RESULT_VULKAN_FAILED;
    }

    if (SyrPipeline_CreatePipeline(*pipeline,
            pipelineCache,
            SyrShaderModule_GetModuleHandle(shaderModule),
            entryPointName,
            shaderPath))
    {
        SyrPipeline_Destroy(*pipeline);
        SyrShaderModule_Destroy(shaderModule);
        *pipeline = NULL;
        return SYR_RESULT_VULKAN_FAILED;
    }

    SyrShaderModule_Destroy(shaderModule);
    return SYR_RESULT_SUCCESS;
}

VkPipeline SyrPipeline_GetPipelineHandle(const SyrPipeline* pipeline)
{
    return pipeline->pipelineHandle;
}

VkPipelineLayout SyrPipeline_GetLayout(const SyrPipeline* pipeline)
{
    return pipeline->layoutHandle;
}

void SyrPipeline_Destroy(SyrPipeline* pipeline)
{
    if (pipeline == NULL)
        return;

    if (pipeline->pipelineHandle != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(pipeline->device, pipeline->pipelineHandle, NULL);
    }

    if (pipeline->layoutHandle != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(pipeline->device, pipeline->layoutHandle, NULL);
    }

    SYR_FREE(pipeline);
}
