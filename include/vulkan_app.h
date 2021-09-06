#ifndef __VULKAN_APP__
#define __VULKAN_APP__ 1

#include <vector>
#include "common_def.h"

struct Context;
struct FrameData;
struct DebugUtils;
struct Resources;
class UserMain;
namespace vkdev {
  class VkTexture;
}

class VulkanApp {

public:
  VulkanApp();
  ~VulkanApp();
  void start();
  void loop();
  void end();


private:
  //INSTANCE
  void createAppInstance();

  //VALIDATION LAYERS
  bool checkValidationLayers();
  void setupDebugMessenger();

  //PHYSICAL DEVICE
  void setupPhysicalDevice();

  //LOGICAL DEVICE
  void createLogicalDevice();

  //SURFACE
  void createSurface();

  //SWAP CHAIN
  void createSwapChain();

  void createPipelineLayout();

  void createPipelineCache();

  //SHADERS //VERTEX //VIEWPORT //DRAW MODE
  void createInternalMaterials();


  //Render Pass
  void createRenderPass();

  //FrameBuffer
  void createFramebuffer();

  //CommandPool
  void createCommandPool();

  void createDepthResource();

  void storeTextures();

  //Buffers
  void createVertexBuffers();
  void createIndexBuffers();

  void createDescriptorSetLayout();
  void createDescriptorPool();
  void createDescriptorSets();
  void createUniformBuffers();

  void updateUniformBuffers(uint32 index);

  void initFrameData(uint32 frame_count);
  void destroyFrameData(FrameData& frame_data);

  int32 acquireNextImage(uint32* image);
  void render(uint32 index);
  void generateBRDFLUT();
  void generateIrradianceCube();
  void generatePrefilteredCube();
  uint32 generateIBLTextures();
  void generateNoiseTexture(uint32 width, uint32 height);
  void updateNoiseTexture(vkdev::VkTexture* texture);

  int32 presentImage(uint32 index);

  //Draw Loop
  void drawFrame();



  VulkanApp(const VulkanApp&);
  Context* context_ = nullptr;
  Resources* resources_;
  DebugUtils* debug_data_ = nullptr;
  UserMain* user_app_ = nullptr;

};

#endif
