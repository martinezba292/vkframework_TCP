#ifndef __VULKAN_APP__
#define __VULKAN_APP__ 1

#include <vector>
#include "common_def.h"

struct Context;
struct FrameData;
struct DebugUtils;
struct UserMain;

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
  std::vector<const char*> getRequiredExtension();

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

  //IMAGE VIEWS
  //void createImageViews();

  //SHADERS //VERTEX //VIEWPORT //DRAW MODE
  void createGraphicsPipeline();

  //Render Pass
  void createRenderPass();

  //FrameBuffer
  void createFramebuffer();

  //CommandPool
  void createCommandPool();

  //Buffers
  void createVertexBuffers();
  void createIndexBuffers();

  void createDescriptorSetLayout();
  void createDescriptorPool();
  void createDescriptorSets();

  void createUniformBuffers();

  void updateUniformBuffers(uint32 index);



  //Command Buffers
  //void createCommandBuffers();
 
  void initFrameData(uint32 frame_count);
  void destroyFrameData(FrameData& frame_data);

  int32 acquireNextImage(uint32* image);

  void render(uint32 index);

  int32 presentImage(uint32 index);

  //Draw Loop
  void drawFrame();


  VulkanApp(const VulkanApp&);
  Context* context_ = nullptr;
  DebugUtils* debug_data_ = nullptr;
  UserMain* user_app_ = nullptr;

};

#endif
