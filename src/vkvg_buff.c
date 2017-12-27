#include "vkvg_buff.h"
#include "vkhelpers.h"

void _set_size_and_map(VkhDevice pDev, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, vkvg_buff *buff){
    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(pDev->vkDev, buff->buffer, &memReq);
    VkMemoryAllocateInfo memAllocInfo = { .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                                          .allocationSize = memReq.size };
    assert(memory_type_from_properties(&pDev->phyMemProps, memReq.memoryTypeBits,memoryPropertyFlags, &memAllocInfo.memoryTypeIndex));
    VK_CHECK_RESULT(vkAllocateMemory(pDev->vkDev, &memAllocInfo, NULL, &buff->memory));

    buff->alignment = memReq.alignment;
    buff->size = memAllocInfo.allocationSize;
    buff->usageFlags = usage;
    buff->memoryPropertyFlags = memoryPropertyFlags;

    VK_CHECK_RESULT(vkBindBufferMemory(buff->pDev->vkDev, buff->buffer, buff->memory, 0));
    VK_CHECK_RESULT(vkMapMemory(buff->pDev->vkDev, buff->memory, 0, VK_WHOLE_SIZE, 0, &buff->mapped));
}

void vkvg_buffer_create(VkhDevice pDev, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, vkvg_buff *buff){
    buff->pDev = pDev;
    VkBufferCreateInfo bufCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .usage = usage, .size = size, .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
    VK_CHECK_RESULT(vkCreateBuffer(pDev->vkDev, &bufCreateInfo, NULL, &buff->buffer));

    _set_size_and_map(pDev,usage,memoryPropertyFlags,size,buff);
}

void vkvg_buffer_destroy(vkvg_buff *buff){
    vkUnmapMemory   (buff->pDev->vkDev, buff->memory);
    vkDestroyBuffer (buff->pDev->vkDev, buff->buffer, NULL);
    vkFreeMemory    (buff->pDev->vkDev, buff->memory, NULL);
}


void vkvg_buffer_increase_size(vkvg_buff *buff, uint32_t sizeAdded){
    size_t oldBSize = buff->size;
    void* pSave = (void*)malloc (oldBSize);
    memcpy (pSave, buff->mapped, oldBSize);

    vkvg_buffer_destroy(buff);
    vkvg_buffer_create(buff->pDev, buff->usageFlags, buff->memoryPropertyFlags, oldBSize + sizeAdded, buff);
    memcpy (buff->mapped, pSave, oldBSize);
    free(pSave);
}
