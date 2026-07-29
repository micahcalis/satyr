#include "VulkInstance.h"
#include "Core/SatyrCore.h"

#define SYR_VALIDATION_LAYERS {"VK_LAYER_KHRONOS_validation"}

#ifdef NDEBUG
static const bool SYR_ENABLE_VALIDATION_LAYERS = false;
    #define SYR_EXTENSION_LAYERS {0}
#else
static const bool SYR_ENABLE_VALIDATION_LAYERS = true;
    #define SYR_EXTENSION_LAYERS {"VK_EXT_debug_utils"}
#endif

typedef struct SyrVulkInstance
{
    VkInstance vkInstanceHandle;
    uint32_t requiredLayersCount;
    uint32_t requiredExtensionsCount;
    const char** requiredLayers;
    const char** requiredExtensions;
    VkDebugUtilsMessengerEXT debugMessenger;
} SyrVulkInstance;

static SyrResult SyrVulkInstance_CreateVkHandle(SyrVulkInstance* vulkInstance)
{
    VkApplicationInfo appInfo = {0};
    appInfo.pApplicationName = "Satyr";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Satyr Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = vulkInstance->requiredExtensionsCount;
    createInfo.ppEnabledExtensionNames = vulkInstance->requiredExtensions;
    createInfo.enabledLayerCount = vulkInstance->requiredLayersCount;
    createInfo.ppEnabledLayerNames = vulkInstance->requiredLayers;

    if (vkCreateInstance(&createInfo, NULL, &vulkInstance->vkInstanceHandle) != VK_SUCCESS)
    {
        SYR_ERROR("Failed to create Vulkan Instance");
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

static SyrResult SyrVulkInstance_InitializeValidationLayers(SyrVulkInstance* vulkInstance)
{
    static const char* requiredLayers[] = SYR_VALIDATION_LAYERS;
    vulkInstance->requiredLayersCount = sizeof(requiredLayers) / sizeof(requiredLayers[0]);
    vulkInstance->requiredLayers = requiredLayers;

    uint32_t availableLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&availableLayerCount, NULL);

    if (availableLayerCount == 0)
    {
        SYR_ERROR("Instance Layer Count is 0!");
        return SYR_RESULT_VULKAN_FAILED;
    }

    VkLayerProperties* availableLayers = SYR_ALLOC_ARRAY(VkLayerProperties, availableLayerCount);
    vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers);

    for (int i = 0; i < vulkInstance->requiredLayersCount; i++)
    {
        bool layerFound = false;

        for (uint32_t j = 0; j < availableLayerCount; ++j)
        {
            if (strcmp(requiredLayers[i], availableLayers[j].layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            free(availableLayers);
            SYR_ERROR("Required Layer not found: %s", requiredLayers[i]);
            return SYR_RESULT_VULKAN_FAILED;
        }
    }

    free(availableLayers);
    return SYR_RESULT_SUCCESS;
}

static SyrResult SyrVulkInstance_InitializeExtensionLayers(SyrVulkInstance* vulkInstance)
{
    static const char* requiredExtensions[] = SYR_EXTENSION_LAYERS;
    vulkInstance->requiredExtensionsCount = sizeof(requiredExtensions) / sizeof(requiredExtensions[0]);
    vulkInstance->requiredExtensions = requiredExtensions;

    if (requiredExtensions[0] == NULL)
    {
        vulkInstance->requiredExtensionsCount = 0;
        vulkInstance->requiredExtensions = NULL;
        return SYR_RESULT_SUCCESS;
    }

    uint32_t availableExtensionsCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionsCount, NULL);

    VkExtensionProperties* availableExtensions = SYR_ALLOC_ARRAY(VkExtensionProperties, availableExtensionsCount);
    vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionsCount, availableExtensions);

    for (int i = 0; i < vulkInstance->requiredExtensionsCount; i++)
    {
        bool layerFound = false;

        for (uint32_t j = 0; j < availableExtensionsCount; ++j)
        {
            if (strcmp(requiredExtensions[i], availableExtensions[j].extensionName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            free(availableExtensions);
            SYR_ERROR("Required Extension not found: %s", requiredExtensions[i]);
            return SYR_RESULT_VULKAN_FAILED;
        }
    }

    free(availableExtensions);
    return SYR_RESULT_SUCCESS;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL SyrVulkInstance_DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    SYR_ERROR("validation layer: type %u msg: %s",
        messageType,
        pCallbackData->pMessage);

    return VK_FALSE;
}

static SyrResult SyrVulkInstance_SetupDebugMessenger(SyrVulkInstance* vulkInstance)
{
    VkDebugUtilsMessageSeverityFlagsEXT severityFlags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    VkDebugUtilsMessageTypeFlagsEXT typeFlags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {0};

    messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerCreateInfo.messageSeverity = severityFlags;
    messengerCreateInfo.messageType = typeFlags;
    messengerCreateInfo.pfnUserCallback = SyrVulkInstance_DebugCallback;

    PFN_vkCreateDebugUtilsMessengerEXT createDebugMessengerFunc = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkInstance->vkInstanceHandle, "vkCreateDebugUtilsMessengerEXT");

    if (createDebugMessengerFunc != NULL)
    {
        if (createDebugMessengerFunc(vulkInstance->vkInstanceHandle, &messengerCreateInfo, NULL, &vulkInstance->debugMessenger) != VK_SUCCESS)
        {
            SYR_ERROR("Failed to create Debug Messenger!");
            return SYR_RESULT_VULKAN_FAILED;
        }
    } else
    {
        SYR_ERROR("vkCreateDebugUtilsMessengerEXT extension function could not be loaded!");
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrVulkInstance_Initialize(const SyrConfig* config, SyrVulkInstance** vulkInstance)
{
    *vulkInstance = SYR_NEW(*vulkInstance);
    (*vulkInstance)->requiredLayersCount = 0;
    (*vulkInstance)->requiredExtensionsCount = 0;

    if (SYR_ENABLE_VALIDATION_LAYERS)
    {
        if (SyrVulkInstance_InitializeValidationLayers(*vulkInstance) == SYR_RESULT_VULKAN_FAILED)
        {
            return SYR_RESULT_VULKAN_FAILED;
        }
    }

    if (SyrVulkInstance_InitializeExtensionLayers(*vulkInstance) == SYR_RESULT_VULKAN_FAILED)
    {
        return SYR_RESULT_VULKAN_FAILED;
    }

    if (SyrVulkInstance_CreateVkHandle(*vulkInstance) == SYR_RESULT_VULKAN_FAILED)
    {
        return SYR_RESULT_VULKAN_FAILED;
    }

    if (SYR_ENABLE_VALIDATION_LAYERS)
    {
        if (SyrVulkInstance_SetupDebugMessenger(*vulkInstance) == SYR_RESULT_VULKAN_FAILED)
        {
            return SYR_RESULT_VULKAN_FAILED;
        }
    }

    return SYR_RESULT_SUCCESS;
}

void SyrVulkInstance_Destroy(SyrVulkInstance* vulkInstance)
{
    if (vulkInstance == NULL)
        return;

    if (SYR_ENABLE_VALIDATION_LAYERS)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugMessengerFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkInstance->vkInstanceHandle, "vkDestroyDebugUtilsMessengerEXT");
        if (destroyDebugMessengerFunc != NULL)
        {
            destroyDebugMessengerFunc(vulkInstance->vkInstanceHandle, vulkInstance->debugMessenger, NULL);
        }
    }

    vkDestroyInstance(vulkInstance->vkInstanceHandle, NULL);

    free(vulkInstance);
}
