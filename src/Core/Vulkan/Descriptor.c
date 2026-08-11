#include "Descriptor.h"

typedef struct SyrDescriptor
{
    VkDescriptorSetLayout layout;
    VkDescriptorSet set;
    uint32_t ssboCount;
    VkDevice device;
    VkDescriptorPool pool;
} SyrDescriptor;

SyrResult SyrDescriptor_Initialize(VkDescriptorSetLayout layout,
    VkDescriptorSet set,
    const uint32_t ssboCount,
    VkDevice device,
    VkDescriptorPool pool,
    SyrDescriptor** descriptor)
{
    *descriptor = SYR_NEW(*descriptor);
    (*descriptor)->layout = layout;
    (*descriptor)->set = set;
    (*descriptor)->ssboCount = ssboCount;
    (*descriptor)->device = device;
    (*descriptor)->pool = pool;

    if (ssboCount > SYR_DESCRIPTOR_SSBO_LIMIT)
    {
        SYR_ERROR("SSBO Count is higher than max limit: 32!");
        SyrDescriptor_Destroy(*descriptor);
        *descriptor = NULL;
        return SYR_RESULT_RUNTIME_ERROR;
    }

    return SYR_RESULT_SUCCESS;
}

static VkDescriptorType SyrDescriptor_GetDescriptorType(const SyrBufferType bufferType)
{
    switch (bufferType)
    {
    case SYR_BUFFER_TYPE_UNIFORM: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case SYR_BUFFER_TYPE_SSBO: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    }
}

void SyrDescriptor_WriteBuffer(SyrDescriptor* descriptor,
    SyrBufferAllocation* bufferAllocation,
    const uint32_t binding,
    const SyrBufferType bufferType,
    const size_t size,
    const size_t offset)
{
    VkDescriptorBufferInfo bufferInfo = {0};
    bufferInfo.buffer = bufferAllocation->bufferHandle;
    bufferInfo.offset = offset;
    bufferInfo.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet descriptorWrite = {0};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptor->set;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = SyrDescriptor_GetDescriptorType(bufferType);
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(descriptor->device,
        1,
        &descriptorWrite,
        0,
        NULL);
}

VkDescriptorSetLayout SyrDescriptor_GetLayout(SyrDescriptor* descriptor)
{
    return descriptor->layout;
}

VkDescriptorSet SyrDescriptor_GetSet(SyrDescriptor* descriptor)
{
    return descriptor->set;
}

uint32_t SyrDescriptor_GetSSBOCount(SyrDescriptor* descriptor)
{
    return descriptor->ssboCount;
}

void SyrDescriptor_Destroy(SyrDescriptor* descriptor)
{
    if (descriptor == NULL)
        return;

    if (descriptor->set != VK_NULL_HANDLE)
    {
        vkFreeDescriptorSets(descriptor->device, descriptor->pool, 1, &descriptor->set);
    }

    if (descriptor->layout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(descriptor->device, descriptor->layout, NULL);
    }

    free(descriptor);
}
