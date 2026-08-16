#include "Device.h"
#include "Core/SatyrCore.h"

typedef struct SyrDevice
{
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VkQueue computeQueues[SYR_QUEUE_PRIORITY_LEVELS];
    uint32_t computeQueueFamilyIndex;
} SyrDevice;

#define SYR_INVALID_COMPUTE_QUEUE UINT32_MAX

static uint32_t GetComputeQueueFamilyIndex(VkPhysicalDevice physicalDevice)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);

    if (queueFamilyCount == 0)
        return SYR_INVALID_COMPUTE_QUEUE;

    VkQueueFamilyProperties* queueFamilies = SYR_ALLOC_ARRAY(VkQueueFamilyProperties, queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);

    bool hasComputeQueue = false;
    uint32_t computeQueueFamilyIndex = SYR_INVALID_COMPUTE_QUEUE;

    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            hasComputeQueue = true;
            computeQueueFamilyIndex = i;
            break;
        }
    }

    free(queueFamilies);

    return computeQueueFamilyIndex;
}

#define SYR_MIN_STORAGE_BUFFERS 32
#define SYR_MIN_DESC_SETS 2

static bool SyrDevice_PhysicalDeviceCompatible(VkPhysicalDevice physicalDevice)
{
    bool hasComputeQueue = GetComputeQueueFamilyIndex(physicalDevice) != SYR_INVALID_COMPUTE_QUEUE;

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    bool vulkanCompatible = deviceProperties.apiVersion >= SYR_VULKAN_VERSION;

    bool ssboCompatible = deviceProperties.limits.maxDescriptorSetStorageBuffers >= SYR_MIN_STORAGE_BUFFERS
        && deviceProperties.limits.maxPerStageDescriptorStorageBuffers >= SYR_MIN_STORAGE_BUFFERS
        && deviceProperties.limits.maxBoundDescriptorSets >= SYR_MIN_DESC_SETS;

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    bool hasSatyrFeatures = deviceFeatures.shaderFloat64
        && deviceFeatures.shaderInt64;

    return hasComputeQueue && vulkanCompatible && ssboCompatible && hasSatyrFeatures;
}

static uint32_t SyrDevice_GetPhysicalDeviceScore(VkPhysicalDevice physicalDevice)
{
    uint32_t score = 0;

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 100000;
    } else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
    {
        score += 20000;
    } else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
    {
        score += 5000;
    }

    score += deviceProperties.limits.maxComputeWorkGroupInvocations;
    score += deviceProperties.limits.maxComputeWorkGroupSize[0];
    score += (deviceProperties.limits.maxComputeSharedMemorySize / 1024);

    return score;
}

static VkPhysicalDevice SyrDevice_GetBestDeviceCandidate(const VkPhysicalDevice* physicalDevices,
    uint32_t* physicalDeviceScores,
    const uint32_t physicalDeviceCount)
{
    VkPhysicalDevice bestCandidate = NULL;
    uint32_t highestScore = 0;

    for (int i = 0; i < physicalDeviceCount; i++)
    {
        uint32_t currentScore = physicalDeviceScores[i];

        if (currentScore > highestScore)
        {
            bestCandidate = physicalDevices[i];
            highestScore = currentScore;
        }
    }

    return bestCandidate;
}

static SyrResult SyrDevice_PickPhysicalDevice(SyrDevice* device,
    SyrVulkInstance* vulkInstance)
{
    uint32_t physicalDeviceCount = 0;

    vkEnumeratePhysicalDevices(SyrVulkInstance_GetInstanceHandle(vulkInstance),
        &physicalDeviceCount,
        NULL);

    if (physicalDeviceCount == 0)
    {
        SYR_ERROR("No Vulkan Compatible Physical Devices found!");
        return SYR_RESULT_VULKAN_FAILED;
    }

    VkPhysicalDevice* physicalDevices = SYR_ALLOC_ARRAY(VkPhysicalDevice, physicalDeviceCount);
    vkEnumeratePhysicalDevices(SyrVulkInstance_GetInstanceHandle(vulkInstance),
        &physicalDeviceCount,
        physicalDevices);

    uint32_t physicalDeviceScores[physicalDeviceCount];
    bool hasCompatibleDevice = false;

    for (int i = 0; i < physicalDeviceCount; i++)
    {
        VkPhysicalDevice currentDevice = physicalDevices[i];
        physicalDeviceScores[i] = 0;

        if (SyrDevice_PhysicalDeviceCompatible(currentDevice))
        {
            hasCompatibleDevice = true;
            physicalDeviceScores[i] += 1000;
            physicalDeviceScores[i] += SyrDevice_GetPhysicalDeviceScore(currentDevice);
        }
    }

    if (!hasCompatibleDevice)
    {
        SYR_ERROR("No Physical Devices Compatible with Saytr!");
        free(physicalDevices);
        return SYR_RESULT_VULKAN_FAILED;
    }

    device->physicalDevice = SyrDevice_GetBestDeviceCandidate(physicalDevices,
        physicalDeviceScores,
        physicalDeviceCount);

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device->physicalDevice, &deviceProperties);
    SYR_LOG("Best Physical Device: %s", deviceProperties.deviceName);

    free(physicalDevices);
    return SYR_RESULT_SUCCESS;
}

static const float SYR_QUEUE_PRIORITY_LOW = 0.0f;
static const float SYR_QUEUE_PRIORITY_MEDIUM = 0.5f;
static const float SYR_QUEUE_PRIORITY_HIGH = 1.0f;

static SyrResult SyrDevice_CreateLogicalDevice(SyrDevice* device)
{
    device->computeQueueFamilyIndex = GetComputeQueueFamilyIndex(device->physicalDevice);

    VkDeviceQueueCreateInfo queueCreateInfo = {0};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = device->computeQueueFamilyIndex;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device->physicalDevice, &queueFamilyCount, NULL);
    VkQueueFamilyProperties* queueFamilies = SYR_ALLOC_ARRAY(VkQueueFamilyProperties, queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device->physicalDevice, &queueFamilyCount, queueFamilies);

    uint32_t availableQueues = queueFamilies[device->computeQueueFamilyIndex].queueCount;
    free(queueFamilies);

    uint32_t requestedQueueCount = SYR_QUEUE_PRIORITY_LEVELS;
    if (requestedQueueCount > availableQueues)
    {
        requestedQueueCount = availableQueues;
    }

    queueCreateInfo.queueCount = requestedQueueCount;

    float queuePriorities[SYR_QUEUE_PRIORITY_LEVELS] = {
        SYR_QUEUE_PRIORITY_HIGH,
        SYR_QUEUE_PRIORITY_MEDIUM,
        SYR_QUEUE_PRIORITY_LOW};

    queueCreateInfo.pQueuePriorities = queuePriorities;

    VkPhysicalDeviceVulkan12Features vulkan12Features = {0};
    vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vulkan12Features.pNext = NULL;
    vulkan12Features.timelineSemaphore = VK_TRUE;

    VkPhysicalDeviceFeatures enabledFeatures = {0};
    enabledFeatures.shaderFloat64 = VK_TRUE;
    enabledFeatures.shaderInt64 = VK_TRUE;

    VkPhysicalDeviceFeatures2 deviceFeatures2 = {0};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.pNext = &vulkan12Features;
    deviceFeatures2.features = enabledFeatures;

    VkDeviceCreateInfo deviceCreateInfo = {0};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = &deviceFeatures2;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.pEnabledFeatures = NULL;

    if (vkCreateDevice(device->physicalDevice,
            &deviceCreateInfo,
            NULL,
            &device->logicalDevice)
        != VK_SUCCESS)
    {
        SYR_ERROR("Failed to create Vulkan Logical Device!");
        return SYR_RESULT_VULKAN_FAILED;
    }

    for (uint32_t i = 0; i < requestedQueueCount; i++)
    {
        vkGetDeviceQueue(device->logicalDevice, device->computeQueueFamilyIndex, i, &device->computeQueues[i]);
    }

    for (uint32_t i = requestedQueueCount; i < SYR_QUEUE_PRIORITY_LEVELS; i++)
    {
        device->computeQueues[i] = device->computeQueues[0];
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrDevice_Initialize(const SyrConfig* config,
    SyrVulkInstance* vulkInstance,
    SyrDevice** device)
{
    if (vulkInstance == NULL)
    {
        SYR_ERROR("Vulkan Instance uninitialized trying to create Device!");
        return SYR_RESULT_VULKAN_FAILED;
    }

    *device = SYR_NEW(*device);

    if (SyrDevice_PickPhysicalDevice(*device, vulkInstance) == SYR_RESULT_VULKAN_FAILED)
    {
        SyrDevice_Destroy(*device);
        *device = NULL;
        return SYR_RESULT_VULKAN_FAILED;
    }

    if (SyrDevice_CreateLogicalDevice(*device) == SYR_RESULT_VULKAN_FAILED)
    {
        SyrDevice_Destroy(*device);
        *device = NULL;
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

VkPhysicalDevice SyrDevice_GetPhysicalDeviceHandle(const SyrDevice* device)
{
    return device->physicalDevice;
}

VkDevice SyrDevice_GetLogicalDeviceHandle(const SyrDevice* device)
{
    return device->logicalDevice;
}

uint32_t SyrDevice_GetComputeFamilyIndex(SyrDevice* device)
{
    return device->computeQueueFamilyIndex;
}

VkQueue SyrDevice_GetComputeQueue(SyrDevice* device, const SyrQueuePriorityLevel level)
{
    return device->computeQueues[(uint32_t)level];
}

void SyrDevice_WaitIdle(SyrDevice* device)
{
    vkDeviceWaitIdle(device->logicalDevice);
}

void SyrDevice_Destroy(SyrDevice* device)
{
    if (device == NULL)
        return;

    if (device->logicalDevice != VK_NULL_HANDLE)
    {
        vkDestroyDevice(device->logicalDevice, NULL);
    }

    free(device);
}
