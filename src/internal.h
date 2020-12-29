#ifndef __INTERNAL_DATA__
#define __INTERNAL_DATA__ 1

#include "common_def.h"
#include "glm/glm.hpp"
#include <chrono>
#include "entity.h"
#include "camera.h"

struct Scene {
  static std::vector<Entity> sceneEntities;
  static std::chrono::steady_clock::time_point lastTime;
  static Camera camera;
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
};

struct SceneUniformBuffer {
  glm::mat4 view;
  glm::mat4 projection;
};

struct Resources {
  std::vector<InternalVertexData> vertex_data;
  VkBuffer bufferObject;
  VkDeviceMemory vertexBufferMemory;
  VkBuffer bufferIndices;
  VkDeviceMemory indexBufferMemory;
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkBuffer> sceneUniformBuffers;
  std::vector<VkDeviceMemory> sceneUniformBufferMemory;
  std::vector<VkDeviceMemory> uniformBufferMemory;
  UniformBufferObject* dynamicUniformData;
};


/********************************************************************************/

struct SwapchainDimension {
  uint32 width = 0;
  uint32 height = 0;
  VkFormat format = VK_FORMAT_UNDEFINED;
  float aspect;
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
  Window* window_;
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

#endif // !1