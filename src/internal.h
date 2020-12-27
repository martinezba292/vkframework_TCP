#ifndef __INTERNAL_DATA__
#define __INTERNAL_DATA__ 1

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "common_def.h"
#include "glm/glm.hpp"
#include <stdexcept>
#include <assert.h>
#include "entity.h"
#include <chrono>


#define MAX_ENTITY 50
struct Scene {
  static std::vector<Entity> sceneEntities;
  static std::chrono::steady_clock::time_point lastTime;
};

struct Vertex;
struct InternalVertexData {
  std::vector<Vertex> vertex;
  std::vector<uint32> indices;
  uint32 offset;
  uint32 index_offset;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = (2 * sizeof(glm::vec3)) + sizeof(glm::vec2);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  static std::vector<VkVertexInputAttributeDescription> getAttributeDescription() {
    std::vector<VkVertexInputAttributeDescription> attributeDescription(3);
    attributeDescription[0].binding = 0;
    attributeDescription[0].location = 0;
    attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription[0].offset = 0;

    attributeDescription[1].binding = 0;
    attributeDescription[1].location = 1;
    attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription[1].offset = sizeof(glm::vec3);

    attributeDescription[2].binding = 0;
    attributeDescription[2].location = 2;
    attributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescription[2].offset = 2 * sizeof(glm::vec3);

    return attributeDescription;
  }
};

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

struct Resources {
  std::vector<InternalVertexData> vertex_data;
  VkBuffer bufferObject;
  VkDeviceMemory vertexBufferMemory;
  VkBuffer bufferIndices;
  VkDeviceMemory indexBufferMemory;
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBufferMemory;
  UniformBufferObject* dynamicUniformData;
};




/********************************************************************************/

struct SwapchainDimension {
  uint32 width = 0;
  uint32 height = 0;
  VkFormat format = VK_FORMAT_UNDEFINED;
};

struct FrameData {
  VkDevice device;
  int32 graphicsQueue;
  VkFence submitFence;
  VkCommandPool primaryCommandPool;
  VkCommandBuffer primaryCommandBuffer;
  VkSemaphore swapchainAcquire;
  VkSemaphore swapchainRelease;
};

struct Context {
  GLFWwindow* window_;
  VkInstance instance_;
  VkPhysicalDevice physDevice_;
  VkDevice logDevice_;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkSwapchainKHR swapChain;
  SwapchainDimension swapchainDimensions;
  VkSurfaceKHR surface;
  std::vector<VkImageView> swapchainImageViews;
  std::vector<VkFramebuffer> swapchainFramebuffers;
  VkRenderPass renderPass;
  VkPipeline pipeline;
  VkPipelineLayout pipelineLayout;
  VkDescriptorSetLayout descriptorLayout;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;
  std::vector<VkSemaphore> recycledSemaphores;
  std::vector<FrameData> perFrame;
  VkCommandPool transferCommandPool;
};


/***************************************************/


//Validation Layers
struct DebugUtils {
  VkDebugUtilsMessengerEXT debugMessenger_;
};

static VkDynamicState dynamicStates[] = {
  VK_DYNAMIC_STATE_VIEWPORT,
  VK_DYNAMIC_STATE_LINE_WIDTH
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

/**********************************************************************************/
//Queue Families
struct QueueFamilyIndices {
  QueueFamilyIndices() : graphicsFamily(-1), presentFamily(-1) {}
  int32 graphicsFamily;
  int32 presentFamily;

  bool isComplete() {
    return graphicsFamily >= 0 && presentFamily >= 0;
  }
};

static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR& surface) {
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

/**********************************************************************************/
//Buffer Memory
static uint32 findMemoryType(VkPhysicalDevice device, uint32 typeFilter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

  for (size_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type");
}


/*Staging Buffers*/
static void createInternalBuffer(Context context, VkDeviceSize size, VkBufferUsageFlags usage, 
                         VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
  
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


static void copyBuffer(Context context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize, VkDeviceSize dstOffset = 0) {
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


static uint64_t padUniformBufferOffset(Context* context, size_t size) {
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(context->physDevice_, &deviceProperties);
  uint64_t minUboAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;

  uint64_t offset = size;
  if (minUboAlignment > 0) {
    offset = (size + minUboAlignment - 1) & ~(minUboAlignment - 1);
  }

  return offset;
}

#endif // !1