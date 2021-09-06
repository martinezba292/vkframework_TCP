#ifndef __INTERNAL_DATA__
#define __INTERNAL_DATA__ 1

#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"
#include <chrono>
#include <array>
#include "draw_cmd.h"
#include "material.h"
#include "vertex_buffer.h"
#include "buffer.h"
#include "dev/ptr_alloc.h"
#include "dev/vktexture.h"
#include <queue>

class Entity;
class Camera;
class Texture;

const uint32 kMaxInstance = 500;
const uint32 kMaxMaterial = 500;
const uint32 kMaxTexture = 20;
const uint32 kTexturePerShader = 10;
const uint32 kMaxLights = 25;

struct Scene {
  static Camera camera;
  static std::array<PtrAlloc<Entity>, kMaxInstance> sceneEntities;
  static std::array<PtrAlloc<Material>, kMaxMaterial> sceneMaterials;
  static std::array<PtrAlloc<Texture>, kMaxTexture> userTextures;
  static uint32 textureCount;
  static uint32 entitiesCount;
  static uint32 materialCount;
  static std::chrono::steady_clock::time_point lastTime;
};

//Used for loadObj function in resource manager
//Need to use Vertex struct with unordered map
template<>struct std::hash<Vertex> {
  size_t operator()(Vertex const& vertex) const {
    return ((hash<glm::vec3>()(vertex.vertex) ^
            (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
            (hash<glm::vec2>()(vertex.uv) << 1);
  }
};

enum class VertexDescriptor {
  kVertexDescriptor_NONE = 0,
  kVertexDescriptor_Pos = 1,
  kVertexDescriptor_Pos_Norm = 2,
  kVertexDescriptor_Pos_Norm_UV = 3,
};

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

  static std::vector<VkVertexInputAttributeDescription> getAttributeDescription(VertexDescriptor desc = 
                                                                                VertexDescriptor::kVertexDescriptor_Pos_Norm_UV) {

    VkFormat formats[3] = { VK_FORMAT_R32G32B32_SFLOAT , 
                            VK_FORMAT_R32G32B32_SFLOAT , 
                            VK_FORMAT_R32G32_SFLOAT };

    int8 desc_index = (int8)desc;
    std::vector<VkVertexInputAttributeDescription> attributeDescription(desc_index);
    for (size_t i = 0; i < desc_index; i++) {
      attributeDescription[i].binding = 0;
      attributeDescription[i].location = i;
      attributeDescription[i].format = formats[i];
      attributeDescription[i].offset = i * sizeof(glm::vec3);
    }

    return attributeDescription;
  }
};

/*****************************************************/

struct UnlitUniform {
  glm::mat4 model;
  glm::vec4 albedo;
};

struct TextureUniform {
  glm::mat4 model;
  uint32 textureIndex;
  glm::vec3 padding;
};

struct BPBRUniform {
  glm::mat4 model;
  glm::vec4 albedo;
  float roughness;
  float metallic;
  glm::vec2 padding;
};

struct SkyboxUniform {
  glm::mat4 viewStatic;
};

struct IBLUniform {
  glm::mat4 model;
  glm::vec4 albedo;
  float roughness;
  float metallic;
  float specular;
  float exposure;
  float gamma;
  glm::vec3 padding;
};

struct NoiseBlock {
  glm::mat4 model;
  float randc;
  float amplification;
  glm::vec2 padding;
};

union UniformBlocks {
  UnlitUniform unlitBlock;
  TextureUniform textureBlock;
  BPBRUniform pbrBlock;
  SkyboxUniform skyboxBlock;
  IBLUniform pbriblBlock;
  NoiseBlock noiseBlock;
};

struct LightParams {
  glm::vec4 lightPosition;
  glm::vec4 lilghtColor;
};

struct SceneUniformBuffer {
  glm::mat4 projection;
  glm::mat4 view;
  std::array<LightParams, kMaxLights> lights;
  glm::vec3 cameraPosition;
  uint32 lightNumber;
};

struct UpdateData {
  glm::mat4 model;
  SceneUniformBuffer sceneBuffer;
  DrawCallData drawCall;
};

enum LayoutType {
  kLayoutType_NONE = -1,
  kLayoutType_Simple_2Binds = 0,
  kLayoutType_Texture_3Binds = 1,
  kLayoutType_Texture_Cubemap,
  kLayoutType_PBRIBL,
  kLayoutType_Noise,
  kLayoutType_MAX
};

struct InternalMaterial {
  VkPipeline matPipeline;
  VkDescriptorPool matDesciptorPool;
  std::vector<VkDescriptorSet> matDescriptorSet;
  LayoutType layout;
  uint32 entitiesReferenced = 0;
  std::vector<uint32> texturesReferenced;
  std::vector<vkdev::Buffer> dynamicUniform;
  UniformBlocks* dynamicUniformData;
};

/*****************************************************/

struct PipelineSettings {
  VkPipelineLayout pipeline;
  VkDescriptorSetLayout descriptor;
};

struct Resources {
  std::vector<InternalVertexData> vertex_data;
  vkdev::Buffer vertexBuffer;
  vkdev::Buffer indicesBuffer;
  std::vector<vkdev::Buffer> staticUniform;

  std::array<PipelineSettings, kLayoutType_MAX> layouts;

  std::array<InternalMaterial, (int32)MaterialType::kMaterialType_MAX> internalMaterials;
  std::vector<vkdev::VkTexture> itextures;
  vkdev::VkTexture depthAttachment;
  std::queue<DrawCallData> draw_calls;
  vkdev::VkTexture brdf;
  vkdev::VkTexture irradianceCube;
  vkdev::VkTexture prefilteredCube;
  vkdev::VkTexture noiseTexture;
  vkdev::VkTexture rockTerrainTexture;
  vkdev::VkTexture grassTerrainTexture;
  VkPipelineCache pipelineCache;
};


/********************************************************************************/

struct SwapchainDimension {
  uint32 width = 0;
  uint32 height = 0;
  VkFormat format = VK_FORMAT_UNDEFINED;
  float aspect = 0.0f;
};

struct FrameData {
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