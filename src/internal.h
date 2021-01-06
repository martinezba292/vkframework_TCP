#ifndef __INTERNAL_DATA__
#define __INTERNAL_DATA__ 1

#include "common_def.h"
#include "glm/glm.hpp"
#include <chrono>
#include <array>
#include "material.h"

class Entity;
class Camera;

const uint32 kMaxInstance = 10;
const uint32 kMaxMaterial = 10;
const uint32 kMaxTexture = 20;
const uint32 kTexturePerShader = 10;

struct Scene {
  static Camera camera;
  static std::array<Entity, kMaxInstance> sceneEntities;
  static std::array<Material, kMaxMaterial> sceneMaterials;
  static std::array<Texture, kMaxTexture> userTextures;
  static uint32 textureCount;
  static uint32 entitiesCount;
  static uint32 materialCount;
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

/*****************************************************/

struct UnlitUniform {
  glm::mat4 model;
  glm::vec4 color;
};

struct TextureUniform {
  glm::mat4 model;
  uint32 textureIndex;
  glm::vec3 padding;
};

union UniformBlocks {
  UnlitUniform unlitBlock;
  TextureUniform textureBlock;
};

enum LayoutType {
  kLayoutType_NONE = -1,
  kLayoutType_Simple_2Binds = 0,
  kLayoutType_Texture_3Binds = 1,
  kLayoutType_MAX
};

struct InternalMaterial {
  VkPipeline matPipeline;
  VkDescriptorPool matDesciptorPool;
  std::vector<VkDescriptorSet> matDescriptorSet;
  LayoutType layout;
  uint32 entitiesReferenced = 0;
  std::vector<uint32> texturesReferenced;
  std::vector<VkBuffer> uniformBuffers;
  std::vector<void*> dynamicUniformMapped;
  std::vector<VkDeviceMemory> uniformBufferMemory;
  UniformBlocks* dynamicUniformData;
};


struct InternalTexture {
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;
};

/*****************************************************/


struct SceneUniformBuffer {
  glm::mat4 view;
  glm::mat4 projection;
};



struct PipelineSettings {
  VkPipelineLayout pipeline;
  VkDescriptorSetLayout descriptor;
};

struct Resources {
  std::vector<InternalVertexData> vertex_data;
  VkBuffer bufferObject;
  VkDeviceMemory vertexBufferMemory;
  VkBuffer bufferIndices;
  VkDeviceMemory indexBufferMemory;
  std::vector<void*> staticUniformMapped;
  std::vector<VkBuffer> sceneUniformBuffers;
  std::vector<VkDeviceMemory> sceneUniformBufferMemory;
  std::array<PipelineSettings, kLayoutType_MAX> layouts;
  std::array<InternalMaterial, kMaterialType_MAX> internalMaterials;
  std::vector<InternalTexture> internalTextures;
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
  std::vector<VkSemaphore> recycledSemaphores;
  std::vector<FrameData> perFrame;
  VkCommandPool transferCommandPool;
  InternalTexture depthAttachment;
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