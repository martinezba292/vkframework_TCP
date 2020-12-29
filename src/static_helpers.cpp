#include "static_helpers.h"
#include "internal.h"
#include <stdexcept>


/***************************************************************************************************/

QueueFamilyIndices StaticHelpers::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR& surface)
{
  uint32 queueFamilyCount = 0;

  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
  QueueFamilyIndices indices;
  int32 i = 0;
  for (const auto& queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }

    VkBool32 presentationSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);
    if (presentationSupport) {
      indices.presentFamily = i;
    }

    if (indices.isComplete()) break;

    ++i;
  }

  return indices;
}

/***************************************************************************************************/

uint32 StaticHelpers::findMemoryType(VkPhysicalDevice device, uint32 typeFilter, VkMemoryPropertyFlags properties)
{
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

  for (size_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type");
}

/***************************************************************************************************/

void StaticHelpers::createInternalBuffer(Context context, VkDeviceSize size, VkBufferUsageFlags usage, 
                                         VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  //assert(vkCreateBuffer(context.logDevice_, &bufferInfo, nullptr, &buffer) == VK_SUCCESS);
  vkCreateBuffer(context.logDevice_, &bufferInfo, nullptr, &buffer);

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(context.logDevice_, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(context.physDevice_,
    memRequirements.memoryTypeBits,
    properties);

  //assert(vkAllocateMemory(context.logDevice_, &allocInfo, nullptr, &bufferMemory) == VK_SUCCESS);
  vkAllocateMemory(context.logDevice_, &allocInfo, nullptr, &bufferMemory);

  vkBindBufferMemory(context.logDevice_, buffer, bufferMemory, 0);
}

/***************************************************************************************************/

void StaticHelpers::copyBuffer(Context context, VkBuffer srcBuffer, VkBuffer dstBuffer, 
                               VkDeviceSize bufferSize, VkDeviceSize dstOffset /*= 0*/)
{
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = context.transferCommandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(context.logDevice_, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = dstOffset;
  copyRegion.size = bufferSize;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(context.graphicsQueue);

  vkFreeCommandBuffers(context.logDevice_, context.transferCommandPool, 1, &commandBuffer);
}

/***************************************************************************************************/

uint64_t StaticHelpers::padUniformBufferOffset(Context* context, size_t size)
{
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(context->physDevice_, &deviceProperties);
  uint64_t minUboAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;

  uint64_t offset = size;
  if (minUboAlignment > 0) {
    offset = (size + minUboAlignment - 1) & ~(minUboAlignment - 1);
  }

  return offset;
}

std::vector<char> StaticHelpers::loadShader(const std::string& filename)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  //assert(file.is_open());
  if (!file.is_open())
    throw(std::runtime_error("Failed to open shader text file"));

  size_t filesize = (size_t)file.tellg();//end of file
  std::vector<char> buffer(filesize);

  file.seekg(0);//return to start
  file.read(buffer.data(), filesize);

  file.close();

  return buffer;
}

VkShaderModule StaticHelpers::createShaderModule(const VkDevice device, const std::vector<char>& code)
{
  VkShaderModuleCreateInfo shader_module_info{};
  shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shader_module_info.codeSize = code.size();
  shader_module_info.pCode = reinterpret_cast<const uint32*>(code.data());

  VkShaderModule shader_module;
  //assert(vkCreateShaderModule(device, &shader_module_info, nullptr, &shader_module) == VK_SUCCESS);
  vkCreateShaderModule(device, &shader_module_info, nullptr, &shader_module);

  return shader_module;
}

SwapChainSupportDetails StaticHelpers::querySwapChain(VkPhysicalDevice device, VkSurfaceKHR surface)
{
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

  uint32 formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
  if (formatCount) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
  }

  uint32 presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
  if (presentModeCount) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

