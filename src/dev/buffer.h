#ifndef __BUFFER_H__
#define __BUFFER_H__ 1

#include "vulkan/vulkan.h"
#include "common_def.h"

struct Context;
namespace vkdev {
  class Buffer {
  public:
    Buffer();
    ~Buffer();
    Buffer(const Buffer&) {}

    int8 createBuffer(Context* context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props);
    int8 copyBuffer(Context* context, Buffer& src_buffer, VkDeviceSize size, VkDeviceSize dst_offset);
    void destroyBuffer();

    void* mapped_;
    VkBuffer buffer_;
    VkDeviceMemory memory_;

  private:
    VkDevice device_;

  };
}

#endif // __BUFFER_H__
