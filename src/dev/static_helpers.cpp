#include "static_helpers.h"
#include "internal.h"
#include <stdexcept>
#include "Components/texture.h"


/***************************************************************************************************/

QueueFamilyIndices dev::StaticHelpers::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR& surface)
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

uint32 dev::StaticHelpers::findMemoryType(VkPhysicalDevice device, uint32 typeFilter, VkMemoryPropertyFlags properties)
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

VkFormat dev::StaticHelpers::findSupportedFormats(Context* context, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
  for (VkFormat format : candidates) {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(context->physDevice_, format, &properties);

    if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
      return format;
    }
    else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("Failed to find supported format");

}

/***************************************************************************************************/

VkFormat dev::StaticHelpers::findDepthFormat(Context* context)
{
  return findSupportedFormats(context,
    { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
    VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
  );
}

/***************************************************************************************************/

uint64_t dev::StaticHelpers::padUniformBufferOffset(Context* context, size_t size)
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

/***************************************************************************************************/

std::vector<char> dev::StaticHelpers::loadShader(const std::string& filename)
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

/***************************************************************************************************/

VkShaderModule dev::StaticHelpers::createShaderModule(const VkDevice device, const std::vector<char>& code)
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

/***************************************************************************************************/

SwapChainSupportDetails dev::StaticHelpers::querySwapChain(VkPhysicalDevice device, VkSurfaceKHR surface)
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

/***************************************************************************************************/

VkPipeline dev::StaticHelpers::createPipeline(Context* context, const char* vert_path, const char* frag_path, VkPipelineLayout pipeline_layout, VkCullModeFlags cull_mode, VkBool32 depth_test)
{
  auto vertex_shader = dev::StaticHelpers::loadShader(vert_path);
  auto fragment_shader = dev::StaticHelpers::loadShader(frag_path);

  VkShaderModule vert_module = dev::StaticHelpers::createShaderModule(context->logDevice_, vertex_shader);
  VkShaderModule frag_module = dev::StaticHelpers::createShaderModule(context->logDevice_, fragment_shader);

  VkPipelineShaderStageCreateInfo vertexShaderInfo{};
  vertexShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertexShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertexShaderInfo.module = vert_module;
  vertexShaderInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragmentShaderInfo{};
  fragmentShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragmentShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragmentShaderInfo.module = frag_module;
  fragmentShaderInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderInfo[]{ vertexShaderInfo, fragmentShaderInfo };


  /*Vertex Input*/
  /*Get vertex data from the resource manager*/
  //const Resources* mainResources = ResourceManager::Get()->getResources();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.pVertexBindingDescriptions = &InternalVertexData::getBindingDescription();
  std::vector<VkVertexInputAttributeDescription> attributeDescription = InternalVertexData::getAttributeDescription();
  vertexInputInfo.vertexAttributeDescriptionCount = attributeDescription.size();
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

  /*Input assembly*/
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  /*Viewport and scissors*/
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)context->swapchainDimensions.width;
  viewport.height = (float)context->swapchainDimensions.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = { 0, 0 };
  scissor.extent.width = context->swapchainDimensions.width;
  scissor.extent.height = context->swapchainDimensions.height;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  /*Rasterizer*/
  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = cull_mode;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;
  rasterizer.depthBiasClamp = 0.0f;
  rasterizer.depthBiasSlopeFactor = 0.0f;

  /*Multisampling*/
  VkPipelineMultisampleStateCreateInfo multisampler{};
  multisampler.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampler.sampleShadingEnable = VK_FALSE;
  multisampler.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampler.minSampleShading = 1.0f;
  multisampler.pSampleMask = nullptr;
  multisampler.alphaToCoverageEnable = VK_FALSE;
  multisampler.alphaToOneEnable = VK_FALSE;

  /*VkPipelineDepthStencilStateCreateInfo depthstencil{};*/

  /*Blend / Transparency*/
  VkPipelineColorBlendAttachmentState blendAttachment{};
  blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  blendAttachment.blendEnable = VK_FALSE;
  blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo blendState{};
  blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blendState.logicOpEnable = VK_FALSE;
  blendState.logicOp = VK_LOGIC_OP_COPY;
  blendState.attachmentCount = 1;
  blendState.pAttachments = &blendAttachment;
  blendState.blendConstants[0] = 0.0f;
  blendState.blendConstants[1] = 0.0f;
  blendState.blendConstants[2] = 0.0f;
  blendState.blendConstants[3] = 0.0f;

  VkPipelineDepthStencilStateCreateInfo depth_stencil{};
  depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_stencil.depthTestEnable = depth_test;
  depth_stencil.depthWriteEnable = depth_test;
  depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depth_stencil.depthBoundsTestEnable = VK_FALSE;
  depth_stencil.minDepthBounds = 0.0f;
  depth_stencil.maxDepthBounds = 1.0f;

  depth_stencil.stencilTestEnable = VK_FALSE;
  depth_stencil.front = {};
  depth_stencil.back = {};

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderInfo;

  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampler;
  pipelineInfo.pDepthStencilState = &depth_stencil;
  pipelineInfo.pColorBlendState = &blendState;
  pipelineInfo.pDynamicState = nullptr;

  pipelineInfo.layout = pipeline_layout;

  pipelineInfo.renderPass = context->renderPass;
  pipelineInfo.subpass = 0;

  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;

  VkPipeline new_pipeline;
  //assert(vkCreateGraphicsPipelines(context_->logDevice_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &context_->pipeline) == VK_SUCCESS);
  vkCreateGraphicsPipelines(context->logDevice_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &new_pipeline);

  vkDestroyShaderModule(context->logDevice_, frag_module, nullptr);
  vkDestroyShaderModule(context->logDevice_, vert_module, nullptr);

  return new_pipeline;
}

/***************************************************************************************************/

VkImageView dev::StaticHelpers::createTextureImageView(VkDevice device, VkImage& image, VkFormat format, VkImageViewType view_type, uint32 mip_levels, uint32 layers, VkImageAspectFlags flags)
{
  //Resources* res = ResourceManager::Get()->getResources();
  /*VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = flags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView image_view_result;
  if (vkCreateImageView(context->logDevice_, &viewInfo, nullptr, &image_view_result) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image view");
  }

  return image_view_result;*/

  VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
  viewInfo.image = image;
  viewInfo.viewType = view_type;
  viewInfo.format = format;
  viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
  viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
  viewInfo.subresourceRange.aspectMask = flags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = mip_levels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = layers;

  VkImageView view;
  VkResult res = vkCreateImageView(device, &viewInfo, nullptr, &view);

  return view;
}
//
///***************************************************************************************************/
//
VkSampler dev::StaticHelpers::createTextureSampler(Context* context, VkSamplerAddressMode address_mode, VkCompareOp compare_op, uint32 mip_levels, VkBorderColor border)
{
  //VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
  //samplerInfo.magFilter = VK_FILTER_LINEAR;
  //samplerInfo.minFilter = VK_FILTER_LINEAR;

  //samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  //samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  //samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  //VkPhysicalDeviceProperties properties{};
  //vkGetPhysicalDeviceProperties(context->physDevice_, &properties);

  //samplerInfo.anisotropyEnable = VK_TRUE;
  //samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

  //samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  //samplerInfo.unnormalizedCoordinates = VK_FALSE;

  //samplerInfo.compareEnable = VK_FALSE;
  //samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

  //samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  //samplerInfo.mipLodBias = 0.0f;
  //samplerInfo.minLod = 0.0f;
  //samplerInfo.maxLod = 0.0f;

  ////Resources* res = ResourceManager::Get()->getResources();

  //VkSampler sampler_result;
  //if (vkCreateSampler(context->logDevice_, &samplerInfo, nullptr, &sampler_result) != VK_SUCCESS) {
  //  throw std::runtime_error("\nFailed to create texture sampler");
  //}

  //return sampler_result;

  VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;

  samplerInfo.addressModeU = address_mode;
  samplerInfo.addressModeV = address_mode;
  samplerInfo.addressModeW = address_mode;

  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(context->physDevice_, &properties);
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

  samplerInfo.borderColor = border;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;

  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = compare_op;

  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = mip_levels;

  VkSampler sampler;
  //assert(vkCreateSampler(device_, &samplerInfo, nullptr, &sampler_) == VK_SUCCESS);
  VkResult res = vkCreateSampler(context->logDevice_, &samplerInfo, nullptr, &sampler);
  return sampler;
}

/***************************************************************************************************/

//void dev::StaticHelpers::createImage(Context* context, uint32 width, uint32 height,
//                                VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
//                                VkMemoryPropertyFlags property, VkImage& image, VkDeviceMemory& image_memory)
//{
//  VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
//  imageInfo.imageType = VK_IMAGE_TYPE_2D;
//  imageInfo.extent.width = static_cast<uint32>(width);
//  imageInfo.extent.height = static_cast<uint32>(height);
//  imageInfo.extent.depth = 1;
//  imageInfo.mipLevels = 1;
//  imageInfo.arrayLayers = 1;
//
//  imageInfo.format = format;
//  imageInfo.tiling = tiling;
//
//  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//  imageInfo.usage = usage;
//
//  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//  imageInfo.flags = 0;
//
//  if (vkCreateImage(context->logDevice_, &imageInfo, nullptr, &image) != VK_SUCCESS) {
//    throw std::runtime_error("\nFailed to create image");
//  }
//
//  VkMemoryRequirements memRequirements;
//  vkGetImageMemoryRequirements(context->logDevice_, image, &memRequirements);
//
//  VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
//  allocInfo.allocationSize = memRequirements.size;
//  allocInfo.memoryTypeIndex = findMemoryType(context->physDevice_, memRequirements.memoryTypeBits, 
//                                             property);
//
//  if (vkAllocateMemory(context->logDevice_, &allocInfo, nullptr, &image_memory) != VK_SUCCESS) {
//    throw std::runtime_error("\Failed to allocate texture memory");
//  }
//
//  vkBindImageMemory(context->logDevice_, image, image_memory, 0);
//}

/***************************************************************************************************/

VkCommandBuffer dev::StaticHelpers::beginSingleTimeCommands(Context* context)
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

/***************************************************************************************************/

void dev::StaticHelpers::endSingleTimeCommands(Context* context, VkCommandBuffer command_buffer)
{
  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &command_buffer;

  vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(context->graphicsQueue);

  vkFreeCommandBuffers(context->logDevice_, context->transferCommandPool, 1, &command_buffer);
}

VkWriteDescriptorSet dev::StaticHelpers::descriptorWriteInitializer(uint32 binding, 
                                                                    VkDescriptorType type, 
                                                                    VkDescriptorSet& descriptor_set,  
                                                                    VkDescriptorBufferInfo* buffer_info,
                                                                    uint32 descriptor_count)
{
  VkWriteDescriptorSet write_descriptor{};
  write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write_descriptor.dstSet = descriptor_set;
  write_descriptor.dstBinding = binding;
  write_descriptor.dstArrayElement = 0;
  write_descriptor.descriptorType = type;
  write_descriptor.descriptorCount = descriptor_count;
  write_descriptor.pTexelBufferView = nullptr;
  write_descriptor.pBufferInfo = buffer_info;
  write_descriptor.pImageInfo = nullptr;
  return write_descriptor;
}


VkWriteDescriptorSet dev::StaticHelpers::descriptorWriteInitializer(uint32 binding, 
                                                                    VkDescriptorType type, 
                                                                    VkDescriptorSet& descriptor_set, 
                                                                    VkDescriptorImageInfo* image_info,
                                                                    uint32 descriptor_count) 
{
  VkWriteDescriptorSet write_descriptor{};
  write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write_descriptor.dstSet = descriptor_set;
  write_descriptor.dstBinding = binding;
  write_descriptor.dstArrayElement = 0;
  write_descriptor.descriptorType = type;
  write_descriptor.descriptorCount = descriptor_count;
  write_descriptor.pTexelBufferView = nullptr;
  write_descriptor.pBufferInfo = nullptr;
  write_descriptor.pImageInfo = image_info;
  return write_descriptor;
}

/***************************************************************************************************/

void dev::StaticHelpers::destroyMaterial(Context* context, InternalMaterial* material)
{
  vkDestroyPipeline(context->logDevice_, material->matPipeline, nullptr);
  for (auto& buffer : material->dynamicUniform) {
    buffer.destroyBuffer();
  }

  vkDestroyDescriptorPool(context->logDevice_, material->matDesciptorPool, nullptr);
  _aligned_free(material->dynamicUniformData);
}


