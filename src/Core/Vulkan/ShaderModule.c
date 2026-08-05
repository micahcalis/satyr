#include "ShaderModule.h"

typedef struct SyrShaderModule
{
    VkShaderModule moduleHandle;
    VkDevice device;
} SyrShaderModule;

static SyrResult SyrShaderModule_OpenFile(const char* fileName,
    size_t* size,
    FILE** file)
{
    errno_t err = fopen_s(file, fileName, "rb");

    if (err != 0 || *file == NULL)
    {
        SYR_ERROR("Could not open file: %s", fileName);
        return SYR_RESULT_RUNTIME_ERROR;
    }

    fseek(*file, 0, SEEK_END);
    long fileSize = ftell(*file);
    *size = (size_t)fileSize;

    return SYR_RESULT_SUCCESS;
}

static SyrResult SyrShaderModule_ReadFileBuffer(const size_t size,
    FILE* file,
    char** buffer)
{
    rewind(file);

    *buffer = (char*)malloc(size);

    if (*buffer == NULL)
    {
        fclose(file);
        SYR_ERROR("Failed allocating file buffer!");
        return SYR_RESULT_RUNTIME_ERROR;
    }

    fread(*buffer, 1, size, file);
    fclose(file);

    return SYR_RESULT_SUCCESS;
}

static SyrResult SyrShaderModule_CreateShaderModule(SyrShaderModule* shaderModule,
    const char* shaderPath,
    char* buffer,
    const size_t size)
{
    VkShaderModuleCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = (uint32_t*)buffer;

    if (vkCreateShaderModule(shaderModule->device,
            &createInfo,
            NULL,
            &shaderModule->moduleHandle)
        != VK_SUCCESS)
    {
        SYR_ERROR("Failed to create Shader Module %s", shaderPath);
        return SYR_RESULT_VULKAN_FAILED;
    }

    free(buffer);

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrShaderModule_Initialize(const char* shaderPath,
    VkDevice device,
    SyrShaderModule** shaderModule)
{
    *shaderModule = SYR_NEW(*shaderModule);
    (*shaderModule)->device = device;
    size_t size = 0;
    FILE* file = NULL;
    char* buffer = NULL;

    if (SyrShaderModule_OpenFile(shaderPath, &size, &file) != SYR_RESULT_SUCCESS)
    {
        SyrShaderModule_Destroy(*shaderModule);
        *shaderModule = NULL;
        return SYR_RESULT_RUNTIME_ERROR;
    }

    if (SyrShaderModule_ReadFileBuffer(size, file, &buffer) != SYR_RESULT_SUCCESS)
    {
        SyrShaderModule_Destroy(*shaderModule);
        *shaderModule = NULL;
        return SYR_RESULT_RUNTIME_ERROR;
    }

    if (SyrShaderModule_CreateShaderModule(*shaderModule, shaderPath, buffer, size))
    {
        SyrShaderModule_Destroy(*shaderModule);
        *shaderModule = NULL;
        return SYR_RESULT_RUNTIME_ERROR;
    }

    return SYR_RESULT_SUCCESS;
}

VkShaderModule SyrShaderModule_GetModuleHandle(SyrShaderModule* shaderModule)
{
    return shaderModule->moduleHandle;
}

void SyrShaderModule_Destroy(SyrShaderModule* shaderModule)
{
    if (shaderModule == NULL)
        return;

    if (shaderModule->moduleHandle != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(shaderModule->device,
            shaderModule->moduleHandle,
            NULL);
    }

    free(shaderModule);
}
