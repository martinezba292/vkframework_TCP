#include "static_helpers.h"
#include "internal.h"
#include <stdexcept>
#include "Components/texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "resource_manager.h"


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
  /*VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = context.transferCommandPool;
  allocInfo.commandBufferCount = 1;*/

  VkCommandBuffer commandBuffer = beginSingleTimeCommands(&context);
  /*vkAllocateCommandBuffers(context.logDevice_, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(commandBuffer, &beginInfo);*/

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = dstOffset;
  copyRegion.size = bufferSize;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  endSingleTimeCommands(&context, commandBuffer);

  /*vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(context.graphicsQueue);

  vkFreeCommandBuffers(context.logDevice_, context.transferCommandPool, 1, &commandBuffer);*/
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

void StaticHelpers::createTextureImage(Context* context, Texture texture)
{
  int32 texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load(texture.getPath().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

  if (!pixels) {
    throw std::runtime_error("\nFailed to load image texture");
  }

  VkDeviceSize imageSize = (uint64_t)(texWidth) * (uint64_t)(texHeight) * 4;
  VkBuffer staging_buffer;
  VkDeviceMemory staging_buffer_memory;
  
  createInternalBuffer(*context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    staging_buffer, staging_buffer_memory);

  void* data;
  vkMapMemory(context->logDevice_, staging_buffer_memory, 0, imageSize, 0, &data);
  memcpy(data, pixels, imageSize);
  vkUnmapMemory(context->logDevice_, staging_buffer_memory);

  stbi_image_free(pixels);

  Resources* res = ResourceManager::Get()->getResources();
  createImage(context, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, 
              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, res->texture.textureImage, res->texture.textureImageMemory);

  transitionImageLayout(context, res->texture.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  copyBufferToImage(context, staging_buffer, res->texture.textureImage, 
                    static_cast<uint32>(texWidth), static_cast<uint32>(texHeight));
  transitionImageLayout(context, res->texture.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(context->logDevice_, staging_buffer, nullptr);
  vkFreeMemory(context->logDevice_, staging_buffer_memory, nullptr);
}

void StaticHelpers::createTextureImageView(Context* context)
{
  Resources* res = ResourceManager::Get()->getResources();
  VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
  viewInfo.image = res->texture.textureImage;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  if (vkCreateImageView(context->logDevice_, &viewInfo, nullptr, &res->texture.textureImageView) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image view");
  }
}

void StaticHelpers::createTextureSampler(Context* context)
{
  VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;

  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(context->physDevice_, &properties);

  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;

  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;

  Resources* res = ResourceManager::Get()->getResources();

  if (vkCreateSampler(context->logDevice_, &samplerInfo, nullptr, &res->texture.textureSampler) != VK_SUCCESS) {
    throw std::runtime_error("\nFailed to create texture sampler");
  }
}

void StaticHelpers::createImage(Context* context, uint32 width, uint32 height, 
                                VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
                                VkMemoryPropertyFlags property, VkImage& image, VkDeviceMemory& image_memory)
{
  VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = static_cast<uint32>(width);
  imageInfo.extent.height = static_cast<uint32>(height);
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;

  imageInfo.format = format;
  imageInfo.tiling = tiling;

  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;

  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0;

  if (vkCreateImage(context->logDevice_, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    throw std::runtime_error("\nFailed to create image");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(context->logDevice_, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(context->physDevice_, memRequirements.memoryTypeBits, 
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  if (vkAllocateMemory(context->logDevice_, &allocInfo, nullptr, &image_memory) != VK_SUCCESS) {
    throw std::runtime_error("\Failed to allocate texture memory");
  }

  vkBindImageMemory(context->logDevice_, image, image_memory, 0);
}

VkCommandBuffer StaticHelpers::beginSingleTimeCommands(Context* context)
{
  VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = context->transferCommandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer cmd_buffer;
  vkAllocateCommandBuffers(context->logDevice_, &allocInfo, &cmd_buffer);

  VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(cmd_buffer, &beginInfo);

  return cmd_buffer;
}

void StaticHelpers::endSingleTimeCommands(Context* context, VkCommandBuffer command_buffer)
{
  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &command_buffer;

  vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(context->graphicsQueue);

  vkFreeCommandBuffers(context->logDevice_, context->transferCommandPool, 1, &command_buffer);
}

void StaticHelpers::transitionImageLayout(Context* context, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
{
  VkCommandBuffer cmd_buffer = beginSingleTimeCommands(context);

  VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;


  VkPipelineStageFlags source_stage;
  VkPipelineStageFlags destination_stage;
  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

  }
  else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;

    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  }
  else {
    throw std::invalid_argument("\nUnsupported layout transition");
  }

  vkCmdPipelineBarrier(cmd_buffer, 0, 0, 0, 0, nullptr, 0, nullptr, 1, &barrier);
  endSingleTimeCommands(context, cmd_buffer);
}

void StaticHelpers::copyBufferToImage(Context* context, VkBuffer buffer, VkImage image, uint32 width, uint32 height)
{
  VkCommandBuffer cmd_buffer = beginSingleTimeCommands(context);

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = { 0, 0, 0 };
  region.imageExtent = { width, height, 1 };

  vkCmdCopyBufferToImage(cmd_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  endSingleTimeCommands(context, cmd_buffer);
}

