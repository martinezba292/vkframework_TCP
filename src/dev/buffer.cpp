#include "buffer.h"
#include <malloc.h>
#include "internal.h"
#include <iostream>
#include "static_helpers.h"


vkdev::Buffer::Buffer()
{
  mapped_ = nullptr;
  device_ = VK_NULL_HANDLE;
  buffer_ = VK_NULL_HANDLE;
  memory_ = VK_NULL_HANDLE;
}

vkdev::Buffer::~Buffer()
{
  destroyBuffer();
}

vkdev::Buffer::Buffer(const Buffer& other)
{
  mapped_ = other.mapped_;
  device_ = other.device_;
  buffer_ = other.buffer_;
  memory_ = other.memory_;
}

int8 vkdev::Buffer::createBuffer(Context* context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props)
{
  if (device_) {
    std::cout << "\nBuffer already created";
    return -1;
  }
  if (!context) {
    std::cout << "\nContext null reference";
    return -1;
  }

  device_ = context->logDevice_;

  VkBufferCreateInfo buffer_info{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  buffer_info.size = size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  vkCreateBuffer(device_, &buffer_info, nullptr, &buffer_);

  VkMemoryRequirements mem_requirements;
  vkGetBufferMemoryRequirements(device_, buffer_, &mem_requirements);

  VkMemoryAllocateInfo alloc_info{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex = dev::StaticHelpers::findMemoryType(context->physDevice_, mem_requirements.memoryTypeBits, props);

  vkAllocateMemory(device_, &alloc_info, nullptr, &memory_);
  vkBindBufferMemory(device_, buffer_, memory_, 0);

  return 0;
}

int8 vkdev::Buffer::copyBuffer(Context* context, Buffer& src_buffer, VkDeviceSize size, VkDeviceSize dst_offset)
{
  if (!context) {
    std::cout << "\nContext null reference";
    return -1;
  }

  VkCommandBuffer cmd_buffer = dev::StaticHelpers::beginSingleTimeCommands(context);

  VkBufferCopy copy_region{};
  copy_region.srcOffset = 0;
  copy_region.dstOffset = dst_offset;
  copy_region.size = size;
  vkCmdCopyBuffer(cmd_buffer, src_buffer.buffer_, buffer_, 1, &copy_region);

  dev::StaticHelpers::endSingleTimeCommands(context, cmd_buffer);
  return 0;
}

void vkdev::Buffer::destroyBuffer()
{
  if (buffer_) {
    vkDestroyBuffer(device_, buffer_, nullptr);
    vkFreeMemory(device_, memory_, nullptr);
  }
}

