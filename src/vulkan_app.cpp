#include "vulkan_app.h"
#include "dev/internal.h"
#include "dev/static_helpers.h"
#include <map>
#include <set>
#include "resource_manager.h"
#include "user_main.h"
#include "Components/geometry.h"
#include "Components/transform.h"
#include <malloc.h>
#include "input_manager.h"
#include "Components/texture.h"
#include "camera.h"
#include "entity.h"
#include "material.h"
#include "Components/point_light.h"
#include "dev/vktexture.h"
#include "glm/gtx/transform.hpp"
#include "perlin_noise.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE


//////////////////////////////////////////////////////////////////////////////////////////////

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData) {

  printf("\n\tVALIDATION_LAYER_MSG:\n%s\n", pCallbackData->pMessage);

  return VK_FALSE;
}

static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerInfo) {

  messengerInfo = {};
  messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

  messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

  messengerInfo.pfnUserCallback = debugCallBack;
  //messengerInfo.pUserData = nullptr; //pUserData parameter of debugCallback function

}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                             const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator,
                                             VkDebugUtilsMessengerEXT* pDebugMessenger) {

  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

  if (func) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  }

  return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
  VkDebugUtilsMessengerEXT debugMessenger,
  const VkAllocationCallbacks* pAllocator) {

  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

  if (func) {
    return func(instance, debugMessenger, pAllocator);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////


/**************************************INSTANCE**********************************************/

void VulkanApp::createAppInstance()
{
  if (enableValidationLayers) {
    assert(checkValidationLayers());
  }
  
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Technical Computing Demo";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "My Vulkan Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  uint32 glfwExtensionCount = 0;
  const char** glfwExtension;
  glfwExtension = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(glfwExtension, glfwExtension + glfwExtensionCount);

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  VkInstanceCreateInfo createInfo{};
  createInfo.enabledExtensionCount = static_cast<uint32>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }
  
  vkCreateInstance(&createInfo, nullptr, &context_->instance_);
}

/***************************************VALIDATION LAYERS************************************************/

bool VulkanApp::checkValidationLayers()
{
  uint32 layerCount = 0;

  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  std::vector<VkLayerProperties> layerProperties(layerCount);

  vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

  for (const char* layerName : validationLayers) {
    bool layerFound = false;

    for (const auto& currentProperty : layerProperties) {
      if (strcmp(currentProperty.layerName, layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) return false;
  }

  return true;
}

void VulkanApp::setupDebugMessenger()
{
  if (!enableValidationLayers) return;

  VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};
  populateDebugMessengerCreateInfo(messengerInfo);
  

  assert(VK_SUCCESS == CreateDebugUtilsMessengerEXT(context_->instance_, &messengerInfo, nullptr, &debug_data_->debugMessenger_));

}

/************************PHYSICAL DEVICE********************************************/

static bool checkDeviceExtensionSupport(VkPhysicalDevice device)
{
  uint32 extensionCount;

  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> avaliableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, avaliableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto& extension : avaliableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

/************************************************************************************************/

static int32 rateDeviceSuitability(VkPhysicalDevice device, Context context) {
  int32 score = 0;
  VkPhysicalDeviceProperties deviceProperties;
  VkPhysicalDeviceFeatures deviceFeatures;

  vkGetPhysicalDeviceProperties(device, &deviceProperties);
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

  if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    score += 1000;

  score += deviceProperties.limits.maxImageDimension2D;

  QueueFamilyIndices indices = dev::StaticHelpers::findQueueFamilies(device, context.surface);
  bool extensionSupported = checkDeviceExtensionSupport(device);
  SwapChainSupportDetails swapChainSupport = dev::StaticHelpers::querySwapChain(device, context.surface);
  bool swapChainSupported = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  if (!indices.isComplete() || !extensionSupported || 
      !swapChainSupported || !deviceFeatures.samplerAnisotropy) {
    return 0;
  }

  return score;
}

/************************************************************************************************/

void VulkanApp::setupPhysicalDevice()
{
  uint32 deviceCount = 0;
  vkEnumeratePhysicalDevices(context_->instance_, &deviceCount, nullptr);
  assert(deviceCount);

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(context_->instance_, &deviceCount, devices.data());

  std::multimap<int32, VkPhysicalDevice> candidates;
  for (const auto& device : devices) {
    int32 score = rateDeviceSuitability(device, *context_);
    candidates.insert(std::make_pair(score, device));
  }

  VkPhysicalDeviceFeatures feat;
  if (candidates.rbegin()->first > 0) {
    context_->physDevice_ = candidates.rbegin()->second;
    context_->physDevice_;
    return;
  }

  abort();
}

/**********************************LOGICAL DEVICE*********************************************/

void VulkanApp::createLogicalDevice()
{
  QueueFamilyIndices indices = dev::StaticHelpers::findQueueFamilies(context_->physDevice_, context_->surface);

  float queuePriority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<int32> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };
  for (int32 queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = queueFamily;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  //vkGetPhysicalDeviceFeatures(context_->physDevice_, &deviceFeatures);

  VkDeviceCreateInfo deviceInfo{};
  deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
  deviceInfo.queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.size());
  deviceInfo.pEnabledFeatures = &deviceFeatures;
  deviceInfo.enabledExtensionCount = static_cast<uint32>(deviceExtensions.size());
  deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();

  if (enableValidationLayers) {
    deviceInfo.enabledLayerCount = static_cast<uint32>(validationLayers.size());
    deviceInfo.ppEnabledLayerNames = validationLayers.data();
  }
  else {
    deviceInfo.enabledLayerCount = 0;
  }
  
  assert(vkCreateDevice(context_->physDevice_, &deviceInfo, nullptr, &context_->logDevice_) == VK_SUCCESS);

  //vkCreateDevice(context_->physDevice_, &deviceInfo, nullptr, &context_->logDevice_) == VK_SUCCESS;

  vkGetDeviceQueue(context_->logDevice_, indices.graphicsFamily, 0, &context_->graphicsQueue);
  vkGetDeviceQueue(context_->logDevice_, indices.presentFamily, 0, &context_->presentQueue);
}


/*********************************WINDOW SURFACE*************************************/

void VulkanApp::createSurface()
{

  assert(glfwCreateWindowSurface(context_->instance_, context_->window_, nullptr, &context_->surface) == VK_SUCCESS);

}

/************************************************************************************************/

void VulkanApp::createSwapChain()
{
  SwapChainSupportDetails swapChainSupport = dev::StaticHelpers::querySwapChain(context_->physDevice_, context_->surface);

  /*SURFACE FORMAT KHR*/
  VkSurfaceFormatKHR surfaceFormat = swapChainSupport.formats[0];
  for (const auto& currentFormat : swapChainSupport.formats) {
    if (currentFormat.format == VK_FORMAT_B8G8R8A8_SRGB && currentFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      surfaceFormat = currentFormat;
    }
  }

  /*PRESENTATION MODE KHR*/
  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
  for (const auto& currentPresentMode : swapChainSupport.presentModes) {
    if (currentPresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      presentMode = currentPresentMode;
    }
  }

  /*SWAP EXTENT*/
  VkExtent2D swapExtent = { k_wWidth, k_wHeight};
  swapExtent.width = std::max(swapChainSupport.capabilities.minImageExtent.width, swapChainSupport.capabilities.maxImageExtent.width);
  swapExtent.height = std::max(swapChainSupport.capabilities.minImageExtent.height, swapChainSupport.capabilities.maxImageExtent.height);


  uint32 swapImageCount = swapChainSupport.capabilities.minImageCount + 1;

  if (swapChainSupport.capabilities.maxImageCount > 0 && swapImageCount > swapChainSupport.capabilities.maxImageCount) {
    swapImageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swapChainInfo{};
  swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapChainInfo.surface = context_->surface;

  swapChainInfo.minImageCount = swapImageCount;
  swapChainInfo.imageFormat = surfaceFormat.format;
  swapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
  swapChainInfo.imageExtent = swapExtent;
  swapChainInfo.imageArrayLayers = 1;
  swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = dev::StaticHelpers::findQueueFamilies(context_->physDevice_, context_->surface);
  uint32 queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

  if (indices.graphicsFamily != indices.presentFamily) {
    swapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapChainInfo.queueFamilyIndexCount = 2;
    swapChainInfo.pQueueFamilyIndices = queueFamilyIndices;
  }
  else {
    swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainInfo.queueFamilyIndexCount = 0;
    swapChainInfo.pQueueFamilyIndices = nullptr;
  }

  swapChainInfo.preTransform = swapChainSupport.capabilities.currentTransform; //VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR
  swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  swapChainInfo.presentMode = presentMode;
  swapChainInfo.clipped = VK_TRUE;

  swapChainInfo.clipped = VK_NULL_HANDLE;

  assert(vkCreateSwapchainKHR(context_->logDevice_, &swapChainInfo, nullptr, &context_->swapChain) == VK_SUCCESS);


  vkGetSwapchainImagesKHR(context_->logDevice_, context_->swapChain, &swapImageCount, nullptr);
  std::vector<VkImage> swap_chain_images(swapImageCount);
  vkGetSwapchainImagesKHR(context_->logDevice_, context_->swapChain, &swapImageCount, swap_chain_images.data());

  context_->swapchainDimensions.format = surfaceFormat.format;
  context_->swapchainDimensions.width = swapExtent.width;
  context_->swapchainDimensions.height = swapExtent.height;


  /*Image Views*/
  context_->perFrame.clear();
  context_->perFrame.resize(swapImageCount);

  initFrameData(swapImageCount);
  context_->swapchainImageViews.resize(swapImageCount);
  for (size_t i = 0; i < swapImageCount; i++) {
    context_->swapchainImageViews[i] = dev::StaticHelpers::createTextureImageView(context_->logDevice_, 
                                                                                  swap_chain_images[i],
                                                                                  surfaceFormat.format, 
                                                                                  VK_IMAGE_VIEW_TYPE_2D, 
                                                                                  1, 1, 
                                                                                  VK_IMAGE_ASPECT_COLOR_BIT);
  }

}

/*********************************************************************************************/

void VulkanApp::createPipelineLayout()
{
  Resources* res = ResourceManager::Get()->getResources();

  for (size_t i = 0; i <  kLayoutType_MAX; i++) {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &res->layouts[i].descriptor;

    vkCreatePipelineLayout(context_->logDevice_, &pipelineLayoutInfo, nullptr, &res->layouts[i].pipeline);
  }
}

/*********************************************************************************************/

void VulkanApp::createPipelineCache()
{
  VkPipelineCacheCreateInfo pipeline_cache_ci{ VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
  VkResult result = vkCreatePipelineCache(context_->logDevice_, &pipeline_cache_ci, nullptr, &resources_->pipelineCache);
}

/*********************************************************************************************/

void VulkanApp::createInternalMaterials()
{
  //Resources* res = ResourceManager::Get()->getResources();
  InternalMaterial* material = &resources_->internalMaterials[(int32)MaterialType::kMaterialType_UnlitColor];
  material->layout = kLayoutType_Simple_2Binds;
  material->matPipeline = dev::StaticHelpers::createPipeline(context_, 
                                                             "./../../src/shaders/spir-v/unlit_color_vert.spv",
                                                             "./../../src/shaders/spir-v/unlit_color_frag.spv",
                                                             resources_->layouts[kLayoutType_Simple_2Binds].pipeline, 
                                                             VK_CULL_MODE_FRONT_BIT, VK_TRUE);

  material = &resources_->internalMaterials[(int32)MaterialType::kMaterialType_BasicPBR];
  material->layout = kLayoutType_Simple_2Binds;
  material->matPipeline = dev::StaticHelpers::createPipeline(context_, 
                                                             "./../../src/shaders/spir-v/basic_pbr_vert.spv",
                                                             "./../../src/shaders/spir-v/basic_pbr_frag.spv",
                                                             resources_->layouts[kLayoutType_Simple_2Binds].pipeline, 
                                                             VK_CULL_MODE_FRONT_BIT, VK_TRUE);

  material = &resources_->internalMaterials[(int32)MaterialType::kMaterialType_TextureSampler];
  material->layout = kLayoutType_Texture_3Binds;
  material->matPipeline = dev::StaticHelpers::createPipeline(context_, 
                                                             "./../../src/shaders/spir-v/texture_sampling_vert.spv",
                                                             "./../../src/shaders/spir-v/texture_sampling_frag.spv",
                                                             resources_->layouts[kLayoutType_Texture_3Binds].pipeline, 
                                                             VK_CULL_MODE_FRONT_BIT, VK_TRUE);

  material = &resources_->internalMaterials[(int32)MaterialType::kMaterialType_Skybox];
  material->layout = kLayoutType_Texture_Cubemap;
  material->matPipeline = dev::StaticHelpers::createPipeline(context_, 
                                                             "./../../src/shaders/spir-v/skybox_vert.spv",
                                                             "./../../src/shaders/spir-v/skybox_frag.spv",
                                                             resources_->layouts[kLayoutType_Texture_Cubemap].pipeline,
                                                             VK_CULL_MODE_BACK_BIT, VK_FALSE);


  material = &resources_->internalMaterials[(int32)MaterialType::kMaterialType_PBRIBL];
  material->layout = kLayoutType_PBRIBL;
  material->matPipeline = dev::StaticHelpers::createPipeline(context_, 
                                                             "./../../src/shaders/spir-v/pbribl_vert.spv",
                                                             "./../../src/shaders/spir-v/pbribl_frag.spv",
                                                             resources_->layouts[kLayoutType_PBRIBL].pipeline,
                                                             VK_CULL_MODE_FRONT_BIT, VK_TRUE, 2);

  material = &resources_->internalMaterials[(int32)MaterialType::kMaterialType_Noise];
  material->layout = kLayoutType_Noise;
  material->matPipeline = dev::StaticHelpers::createPipeline(context_,
                                                             "./../../src/shaders/spir-v/noise_vert.spv",
                                                             "./../../src/shaders/spir-v/noise_frag.spv",
                                                             resources_->layouts[kLayoutType_Noise].pipeline,
                                                             VK_CULL_MODE_FRONT_BIT, VK_TRUE, 3);

}

/*********************************************************************************************/

void VulkanApp::createRenderPass()
{
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = context_->swapchainDimensions.format;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


  VkAttachmentDescription depth_attachment{};
  depth_attachment.format = dev::StaticHelpers::findDepthFormat(context_);
  depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_ref{};
  depth_ref.attachment = 1;
  depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depth_ref;

  /*SUBPASS*/

  std::array<VkSubpassDependency, 2> dependencies;

  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  std::array<VkAttachmentDescription, 2> attachments{ colorAttachment, depth_attachment };

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = dependencies.size();
  renderPassInfo.pDependencies = dependencies.data();

  
#ifdef NDEBUG
  vkCreateRenderPass(context_->logDevice_, &renderPassInfo, nullptr, &context_->renderPass);
#else
  assert(vkCreateRenderPass(context_->logDevice_, &renderPassInfo, nullptr, &context_->renderPass) == VK_SUCCESS);
#endif
}

/*********************************************************************************************/

void VulkanApp::createFramebuffer()
{
  int32 imageCount = context_->swapchainImageViews.size();
  context_->swapchainFramebuffers.resize(imageCount);
  for (size_t i = 0; i < imageCount; i++) {
    std::array<VkImageView, 2> attachment = { context_->swapchainImageViews[i], resources_->depthAttachment.view_ };
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = context_->renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32>(attachment.size());
    framebufferInfo.pAttachments = attachment.data();
    framebufferInfo.width = context_->swapchainDimensions.width;
    framebufferInfo.height = context_->swapchainDimensions.height;
    framebufferInfo.layers = 1;

#ifdef NDEBUG
    vkCreateFramebuffer(context_->logDevice_, &framebufferInfo, nullptr, &context_->swapchainFramebuffers[i]);
#else
    assert(vkCreateFramebuffer(context_->logDevice_, &framebufferInfo, nullptr, &context_->swapchainFramebuffers[i]) == VK_SUCCESS);
#endif
  }
}

/*********************************************************************************************/

void VulkanApp::createCommandPool()
{
  QueueFamilyIndices queueIndices = dev::StaticHelpers::findQueueFamilies(context_->physDevice_, context_->surface);

  VkCommandPoolCreateInfo commandPoolInfo{};
  commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolInfo.queueFamilyIndex = queueIndices.graphicsFamily;

  commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
#ifdef NDEBUG
  vkCreateCommandPool(context_->logDevice_, &commandPoolInfo, nullptr, &context_->transferCommandPool);
#else
  assert(vkCreateCommandPool(context_->logDevice_, &commandPoolInfo, nullptr, &context_->transferCommandPool) == VK_SUCCESS);
#endif
}

/*********************************************************************************************/

void VulkanApp::createDepthResource()
{
  VkFormat depth_format = dev::StaticHelpers::findDepthFormat(context_);

  vkdev::VkTexture* depth = &resources_->depthAttachment;
  depth->device_ = context_->logDevice_;
  depth->width_ = k_wWidth;
  depth->height_ = k_wHeight;
  depth->createImage(context_->physDevice_, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 1, 0);
  depth->view_ = dev::StaticHelpers::createTextureImageView(context_->logDevice_, 
                                                            depth->image_, 
                                                            depth_format, 
                                                            VK_IMAGE_VIEW_TYPE_2D, 
                                                            1, 1, 
                                                            VK_IMAGE_ASPECT_DEPTH_BIT);
}

/*********************************************************************************************/

void VulkanApp::storeTextures()
{
  uint32 textures_number = Scene::textureCount;
  resources_->itextures.resize(textures_number);

  for (size_t i = 0; i < textures_number; i++) {
    Texture* user_texture = Scene::userTextures[i].get();
    //vkdev::VkTexture new_texture;
    if (user_texture->getType() == TextureType::kTextureType_Cubemap)
      resources_->itextures[i].loadCubemapKtx(context_, 
                                              user_texture->getPath().c_str(), 
                                              dev::StaticHelpers::getTextureFormat(user_texture->getFormat()), 
                                              6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, VK_IMAGE_VIEW_TYPE_CUBE);
    else
      resources_->itextures[i].loadImage(context_, 
                                         user_texture->getPath().c_str(), 
                                         dev::StaticHelpers::getTextureFormat(user_texture->getFormat()));
  }
}

/*********************************************************************************************/

void VulkanApp::createVertexBuffers()
{
  Resources* mainResources = ResourceManager::Get()->getResources();
  VkDeviceSize total_vertex_bytes = 0;
  uint32 vertex_offset = 0;
  size_t geometry_number = mainResources->vertex_data.size();
  std::vector<VkDeviceSize> sizes(geometry_number);
  std::vector<VkDeviceSize> offset_bytes(geometry_number);
  InternalVertexData* vertex_data = mainResources->vertex_data.data();
  for (size_t i = 0; i < geometry_number; i++) {
    InternalVertexData* current_vertex = &vertex_data[i];
    current_vertex->offset = vertex_offset;
    vertex_offset += current_vertex->vertex.size();

    sizes[i] = (static_cast<uint64_t>(sizeof(Vertex)) * 
                        current_vertex->vertex.size());
    offset_bytes[i] = total_vertex_bytes;
    total_vertex_bytes += sizes[i];
  }
  mainResources->vertexBuffer.createBuffer(context_, total_vertex_bytes, 
                                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  for (size_t i = 0; i < geometry_number; i++) {
    InternalVertexData* current_vertex = &vertex_data[i];

    VkDeviceSize size = sizes[i];
    vkdev::Buffer staging_buffer;
    staging_buffer.createBuffer(context_, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(context_->logDevice_, staging_buffer.memory_, 0, size, 0, &data);
    memcpy(data, current_vertex->vertex.data(), size);
    vkUnmapMemory(context_->logDevice_, staging_buffer.memory_);

    mainResources->vertexBuffer.copyBuffer(context_, staging_buffer, size, offset_bytes[i]);
  }
}

/*********************************************************************************************/

void VulkanApp::createIndexBuffers()
{
  Resources* mainResources = ResourceManager::Get()->getResources();
  VkDeviceSize total_index_bytes = 0;
  uint32 index_offset = 0;
  size_t geometry_number = mainResources->vertex_data.size();
  std::vector<VkDeviceSize> sizes(geometry_number);
  std::vector<VkDeviceSize> offset_bytes(geometry_number);
  for (size_t i = 0; i < geometry_number; i++) {
    mainResources->vertex_data[i].index_offset = index_offset;
    index_offset += mainResources->vertex_data[i].indices.size();

    sizes[i] = (static_cast<uint64_t>(sizeof(uint32)) * mainResources->vertex_data[i].indices.size());
    offset_bytes[i] = total_index_bytes;
    total_index_bytes += sizes[i];
  }
  mainResources->indicesBuffer.createBuffer(context_, total_index_bytes, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  for (size_t i = 0; i < geometry_number; i++) {
    InternalVertexData* vertexData = &mainResources->vertex_data[i];

    VkDeviceSize size = sizes[i];
    vkdev::Buffer staging_buffer;
    staging_buffer.createBuffer(context_, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(context_->logDevice_, staging_buffer.memory_, 0, size, 0, &data);
    memcpy(data, vertexData->indices.data(), size);
    vkUnmapMemory(context_->logDevice_, staging_buffer.memory_);

    mainResources->indicesBuffer.copyBuffer(context_, staging_buffer, size, offset_bytes[i]);
    //staging_buffer.destroyBuffer();
  }
}

/*********************************************************************************************/

void VulkanApp::generateBRDFLUT()
{
  VkFormat format = VK_FORMAT_R16G16_SFLOAT;
  int32 dim = 512;

  //resources_->brdf.alloc();
  vkdev::VkTexture* brdfTexture = &resources_->brdf;
  brdfTexture->device_ = context_->logDevice_;
  brdfTexture->width_ = dim;
  brdfTexture->height_ = dim;
  brdfTexture->createImage(context_->physDevice_, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 0);
  //VkComponentMapping comp = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
  brdfTexture->view_ = dev::StaticHelpers::createTextureImageView(context_->logDevice_, brdfTexture->image_, format, VK_IMAGE_VIEW_TYPE_2D, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT);
  brdfTexture->sampler_ = dev::StaticHelpers::createTextureSampler(context_, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_COMPARE_OP_NEVER, 1, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, VK_FALSE);


  brdfTexture->descriptor_ = {};
  brdfTexture->descriptor_.imageView = brdfTexture->view_;
  brdfTexture->descriptor_.sampler = brdfTexture->sampler_;
  brdfTexture->descriptor_.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkAttachmentDescription attDesc = {};
  // Color attachment
  attDesc.format = format;
  attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
  attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

  VkSubpassDescription subpassDescription = {};
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pColorAttachments = &colorReference;

  std::array<VkSubpassDependency, 2> dependencies;
  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo renderPassCI{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  renderPassCI.attachmentCount = 1;
  renderPassCI.pAttachments = &attDesc;
  renderPassCI.subpassCount = 1;
  renderPassCI.pSubpasses = &subpassDescription;
  renderPassCI.dependencyCount = 2;
  renderPassCI.pDependencies = dependencies.data();


  VkRenderPass renderpass;
  VkResult result = vkCreateRenderPass(context_->logDevice_, &renderPassCI, nullptr, &renderpass);

  VkFramebufferCreateInfo framebufferCI{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
  framebufferCI.renderPass = renderpass;
  framebufferCI.attachmentCount = 1;
  framebufferCI.pAttachments = &brdfTexture->view_;
  framebufferCI.width = dim;
  framebufferCI.height = dim;
  framebufferCI.layers = 1;

  VkFramebuffer framebuffer;
  result = vkCreateFramebuffer(context_->logDevice_, &framebufferCI, nullptr, &framebuffer);

  // Descriptors
  VkDescriptorSetLayout descriptorsetlayout;
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {};
  VkDescriptorSetLayoutCreateInfo descriptorsetlayoutCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
  descriptorsetlayoutCI.pBindings = setLayoutBindings.data();
  descriptorsetlayoutCI.bindingCount = static_cast<uint32>(setLayoutBindings.size());
  result = vkCreateDescriptorSetLayout(context_->logDevice_, &descriptorsetlayoutCI, nullptr, &descriptorsetlayout);

  // Descriptor Pool
  std::vector<VkDescriptorPoolSize> pool_sizes{ {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1} };

  VkDescriptorPoolCreateInfo descriptorPoolCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
  descriptorPoolCI.pPoolSizes = pool_sizes.data();
  descriptorPoolCI.poolSizeCount = static_cast<uint32>(pool_sizes.size());
  descriptorPoolCI.maxSets = 2;
  VkDescriptorPool descriptor_pool;
  result = vkCreateDescriptorPool(context_->logDevice_, &descriptorPoolCI, nullptr, &descriptor_pool);

  // Descriptor sets
  VkDescriptorSet descriptorset;
  VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
  allocInfo.descriptorPool = descriptor_pool;
  allocInfo.pSetLayouts = &descriptorsetlayout;
  allocInfo.descriptorSetCount = 1;
  result = vkAllocateDescriptorSets(context_->logDevice_, &allocInfo, &descriptorset);

  // Pipeline layout
  VkPipelineLayout pipelinelayout;
  VkPipelineLayoutCreateInfo pipelineLayoutCI{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
  pipelineLayoutCI.pSetLayouts = &descriptorsetlayout;
  pipelineLayoutCI.setLayoutCount = 1;
  result = vkCreatePipelineLayout(context_->logDevice_, &pipelineLayoutCI, nullptr, &pipelinelayout);
  




  //// Pipeline
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
  inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssemblyState.flags = 0;
  inputAssemblyState.primitiveRestartEnable = VK_FALSE;

  VkPipelineRasterizationStateCreateInfo rasterizationState{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
  rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationState.cullMode = VK_CULL_MODE_NONE;
  rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizationState.flags = 0;
  rasterizationState.depthClampEnable = VK_FALSE;
  rasterizationState.lineWidth = 1.0f;
  //
  VkPipelineColorBlendAttachmentState blendAttachmentState{};
  blendAttachmentState.colorWriteMask = 0xf;
  blendAttachmentState.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlendState{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
  colorBlendState.attachmentCount = 1;
  colorBlendState.pAttachments = &blendAttachmentState;

  VkPipelineDepthStencilStateCreateInfo depthStencilState{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
  depthStencilState.depthTestEnable = VK_FALSE;
  depthStencilState.depthWriteEnable = VK_FALSE;
  depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

  VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;
  viewportState.flags = 0;

  VkPipelineMultisampleStateCreateInfo multisampleState{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
  multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampleState.flags = 0;

  std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
  VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
  dynamicState.pDynamicStates = dynamicStateEnables.data();
  dynamicState.dynamicStateCount = static_cast<uint32>(dynamicStateEnables.size());
  dynamicState.flags = 0;

  VkPipelineVertexInputStateCreateInfo emptyInputState{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

  //std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

  VkGraphicsPipelineCreateInfo pipelineCI{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
  pipelineCI.layout = pipelinelayout;
  pipelineCI.renderPass = renderpass;
  pipelineCI.basePipelineIndex = -1;
  pipelineCI.basePipelineHandle = VK_NULL_HANDLE;

  pipelineCI.pInputAssemblyState = &inputAssemblyState;
  pipelineCI.pRasterizationState = &rasterizationState;
  pipelineCI.pColorBlendState = &colorBlendState;
  pipelineCI.pMultisampleState = &multisampleState;
  pipelineCI.pViewportState = &viewportState;
  pipelineCI.pDepthStencilState = &depthStencilState;
  pipelineCI.pDynamicState = &dynamicState;
  pipelineCI.pVertexInputState = &emptyInputState;


  auto vertex_shader = dev::StaticHelpers::loadShader("./../../src/shaders/spir-v/genbrdflut_vert.spv");
  auto fragment_shader = dev::StaticHelpers::loadShader("./../../src/shaders/spir-v/genbrdflut_frag.spv");
  
  VkShaderModule vert_module = dev::StaticHelpers::createShaderModule(context_->logDevice_, vertex_shader);
  VkShaderModule frag_module = dev::StaticHelpers::createShaderModule(context_->logDevice_, fragment_shader);
  
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
  pipelineCI.stageCount = 2;
  pipelineCI.pStages = shaderInfo;


  VkPipeline pipeline;
  vkCreateGraphicsPipelines(context_->logDevice_, resources_->pipelineCache, 1, &pipelineCI, nullptr, &pipeline);

  vkDestroyShaderModule(context_->logDevice_, vert_module, nullptr);
  vkDestroyShaderModule(context_->logDevice_, frag_module, nullptr);

  // Render
  VkClearValue clearValues[1];
  clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

  VkRenderPassBeginInfo renderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
  renderPassBeginInfo.renderPass = renderpass;
  renderPassBeginInfo.renderArea.extent.width = dim;
  renderPassBeginInfo.renderArea.extent.height = dim;
  renderPassBeginInfo.clearValueCount = 1;
  renderPassBeginInfo.pClearValues = clearValues;
  renderPassBeginInfo.framebuffer = framebuffer;

  VkCommandBuffer cmdBuf = dev::StaticHelpers::beginSingleTimeCommands(context_);
  vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
  VkViewport viewport{};
  viewport.width = (float)dim;
  viewport.height = (float)dim;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.extent.width = dim;
  scissor.extent.height = dim;
  scissor.offset.x = 0;
  scissor.offset.y = 0;

  vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
  vkCmdSetScissor(cmdBuf, 0, 1, &scissor);
  vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  vkCmdDraw(cmdBuf, 3, 1, 0, 0);
  vkCmdEndRenderPass(cmdBuf);
  dev::StaticHelpers::endSingleTimeCommands(context_, cmdBuf);

  vkDestroyPipeline(context_->logDevice_, pipeline, nullptr);
  vkDestroyPipelineLayout(context_->logDevice_, pipelinelayout, nullptr);
  vkDestroyRenderPass(context_->logDevice_, renderpass, nullptr);
  vkDestroyFramebuffer(context_->logDevice_, framebuffer, nullptr);
  vkDestroyDescriptorSetLayout(context_->logDevice_, descriptorsetlayout, nullptr);
  vkDestroyDescriptorPool(context_->logDevice_, descriptor_pool, nullptr);
}


void VulkanApp::generateIrradianceCube()
{
  const VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;
  const int32_t dim = 64;
  const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;

  //resources_->irradianceCube.alloc();
  vkdev::VkTexture* irradianceCube = &resources_->irradianceCube;
  irradianceCube->width_ = dim;
  irradianceCube->height_ = dim;
  irradianceCube->mipLevels_ = numMips;
  irradianceCube->device_ = context_->logDevice_;
  irradianceCube->createImage(context_->physDevice_, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
  //VkComponentMapping comp = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
  irradianceCube->view_ = dev::StaticHelpers::createTextureImageView(context_->logDevice_, irradianceCube->image_, format, VK_IMAGE_VIEW_TYPE_CUBE, numMips, 6, VK_IMAGE_ASPECT_COLOR_BIT);
  irradianceCube->sampler_ = dev::StaticHelpers::createTextureSampler(context_, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_COMPARE_OP_NEVER, numMips, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, VK_FALSE);

  irradianceCube->descriptor_ = {};
  irradianceCube->descriptor_.imageView = irradianceCube->view_;
  irradianceCube->descriptor_.sampler = irradianceCube->sampler_;
  irradianceCube->descriptor_.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkAttachmentDescription attDesc = {};
  // Color attachment
  attDesc.format = format;
  attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
  attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

  VkSubpassDescription subpassDescription = {};
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pColorAttachments = &colorReference;

  std::array<VkSubpassDependency, 2> dependencies;
  //dependencies[0] = {};
  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
  //dependencies[1] = {};
  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo renderPassCI{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  renderPassCI.attachmentCount = 1;
  renderPassCI.pAttachments = &attDesc;
  renderPassCI.subpassCount = 1;
  renderPassCI.pSubpasses = &subpassDescription;
  renderPassCI.dependencyCount = 2;
  renderPassCI.pDependencies = dependencies.data();

  VkRenderPass renderpass;
  vkCreateRenderPass(context_->logDevice_, &renderPassCI, nullptr, &renderpass);

  vkdev::VkTexture offscreentexture;
  //offscreentexture.alloc();
  offscreentexture.width_ = dim;
  offscreentexture.height_ = dim;
  offscreentexture.mipLevels_ = 1;
  offscreentexture.device_ = context_->logDevice_;
  offscreentexture.createImage(context_->physDevice_, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 1, 0);
  offscreentexture.view_ = dev::StaticHelpers::createTextureImageView(context_->logDevice_, offscreentexture.image_, format, VK_IMAGE_VIEW_TYPE_2D, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT);

  VkFramebuffer offscreen_framebuffer;

  VkFramebufferCreateInfo fbufCreateInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
  fbufCreateInfo.renderPass = renderpass;
  fbufCreateInfo.attachmentCount = 1;
  fbufCreateInfo.pAttachments = &offscreentexture.view_;
  fbufCreateInfo.width = dim;
  fbufCreateInfo.height = dim;
  fbufCreateInfo.layers = 1;
  vkCreateFramebuffer(context_->logDevice_, &fbufCreateInfo, nullptr, &offscreen_framebuffer);

  VkImageSubresourceRange subresourceRange{};
  subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount = 1;
  subresourceRange.layerCount = 1;
  VkCommandBuffer cmd_buffer = dev::StaticHelpers::beginSingleTimeCommands(context_);
  offscreentexture.setImageLayout(cmd_buffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, subresourceRange);
  dev::StaticHelpers::endSingleTimeCommands(context_, cmd_buffer);
  
  VkDescriptorSetLayout descriptorsetlayout;
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
    dev::StaticHelpers::layoutBindingInitializer(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)
  };

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = dev::StaticHelpers::setLayoutCreateInfoInitializer(setLayoutBindings);
  vkCreateDescriptorSetLayout(context_->logDevice_, &descriptorSetLayoutCI, nullptr, &descriptorsetlayout);

  // Descriptor Pool
  std::vector<VkDescriptorPoolSize> pool_sizes{ {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1} };

  VkDescriptorPoolCreateInfo descriptorPoolCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
  descriptorPoolCI.pPoolSizes = pool_sizes.data();
  descriptorPoolCI.poolSizeCount = static_cast<uint32>(pool_sizes.size());
  descriptorPoolCI.maxSets = 2;
  VkDescriptorPool descriptor_pool;
  vkCreateDescriptorPool(context_->logDevice_, &descriptorPoolCI, nullptr, &descriptor_pool);

  VkDescriptorSet descriptorset;
  VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
  allocInfo.descriptorPool = descriptor_pool;
  allocInfo.pSetLayouts = &descriptorsetlayout;
  allocInfo.descriptorSetCount = 1;
  vkAllocateDescriptorSets(context_->logDevice_, &allocInfo, &descriptorset);

  VkWriteDescriptorSet writeDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
  writeDescriptorSet.dstSet = descriptorset;
  writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  writeDescriptorSet.dstBinding = 0;
  writeDescriptorSet.descriptorCount = 1;
  ///////////////////////////////UPDATE IMAGE INFO/////////////////////////////////////
  uint32 skybox = resources_->internalMaterials[(uint32)MaterialType::kMaterialType_Skybox].texturesReferenced[0];
  writeDescriptorSet.pImageInfo = &resources_->itextures[skybox].descriptor_;

  vkUpdateDescriptorSets(context_->logDevice_, 1, &writeDescriptorSet, 0, nullptr);


  // Pipeline layout
  struct PushBlock {
    glm::mat4 mvp;
    // Sampling deltas
    float deltaPhi = (2.0f * PI) / 180.0f;
    float deltaTheta = (0.5f * PI) / 64.0f;
  } pushBlock;

  VkPipelineLayout pipelinelayout;
  std::vector<VkPushConstantRange> pushConstantRanges = {
    {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlock)},
  };
  
  VkPipelineLayoutCreateInfo pipelineLayoutCI{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
  pipelineLayoutCI.setLayoutCount = 1;
  pipelineLayoutCI.pSetLayouts = &descriptorsetlayout;
  pipelineLayoutCI.pushConstantRangeCount = 1;
  pipelineLayoutCI.pPushConstantRanges = pushConstantRanges.data();
  vkCreatePipelineLayout(context_->logDevice_, &pipelineLayoutCI, nullptr, &pipelinelayout);

  // Pipeline
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
  inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssemblyState.primitiveRestartEnable = VK_FALSE;

  VkPipelineRasterizationStateCreateInfo rasterizationState{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
  rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationState.cullMode = VK_CULL_MODE_NONE;
  rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizationState.depthClampEnable = VK_FALSE;
  rasterizationState.lineWidth = 1.0f;

  VkPipelineColorBlendAttachmentState blendAttachmentState{};
  blendAttachmentState.colorWriteMask = 0xf;
  blendAttachmentState.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlendState{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
  colorBlendState.attachmentCount = 1;
  colorBlendState.pAttachments = &blendAttachmentState;

  VkPipelineDepthStencilStateCreateInfo depthStencilState{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
  depthStencilState.depthTestEnable = VK_FALSE;
  depthStencilState.depthWriteEnable = VK_FALSE;
  depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

  VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;
  viewportState.flags = 0;

  VkPipelineMultisampleStateCreateInfo multisampleState{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
  multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
  VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
  dynamicState.pDynamicStates = dynamicStateEnables.data();
  dynamicState.dynamicStateCount = static_cast<uint32>(dynamicStateEnables.size());

  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

  VkGraphicsPipelineCreateInfo pipelineCI{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
  pipelineCI.layout = pipelinelayout;
  pipelineCI.renderPass = renderpass;
  pipelineCI.basePipelineIndex = -1;
  pipelineCI.basePipelineHandle = VK_NULL_HANDLE;

  pipelineCI.pInputAssemblyState = &inputAssemblyState;
  pipelineCI.pRasterizationState = &rasterizationState;
  pipelineCI.pColorBlendState = &colorBlendState;
  pipelineCI.pMultisampleState = &multisampleState;
  pipelineCI.pViewportState = &viewportState;
  pipelineCI.pDepthStencilState = &depthStencilState;
  pipelineCI.pDynamicState = &dynamicState;
  pipelineCI.stageCount = 2;
  pipelineCI.pStages = shaderStages.data();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.pVertexBindingDescriptions = &InternalVertexData::getBindingDescription();
  std::vector<VkVertexInputAttributeDescription> attributeDescription = InternalVertexData::getAttributeDescription(VertexDescriptor::kVertexDescriptor_Pos);
  vertexInputInfo.vertexAttributeDescriptionCount = attributeDescription.size();
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

  pipelineCI.pVertexInputState = &vertexInputInfo;

  auto vertex_shader = dev::StaticHelpers::loadShader("./../../src/shaders/spir-v/filtercube_vert.spv");
  auto fragment_shader = dev::StaticHelpers::loadShader("./../../src/shaders/spir-v/irradiancecube_frag.spv");

  VkShaderModule vert_module = dev::StaticHelpers::createShaderModule(context_->logDevice_, vertex_shader);
  VkShaderModule frag_module = dev::StaticHelpers::createShaderModule(context_->logDevice_, fragment_shader);

  shaderStages[0] = {};
  shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStages[0].module = vert_module;
  shaderStages[0].pName = "main";

  shaderStages[1] = {};
  shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStages[1].module = frag_module;
  shaderStages[1].pName = "main";

  VkPipeline pipeline;
  vkCreateGraphicsPipelines(context_->logDevice_, resources_->pipelineCache, 1, &pipelineCI, nullptr, &pipeline);

  vkDestroyShaderModule(context_->logDevice_, vert_module, nullptr);
  vkDestroyShaderModule(context_->logDevice_, frag_module, nullptr);

  // Render
  VkClearValue clearValues[1];
  clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };

  VkRenderPassBeginInfo renderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
  renderPassBeginInfo.renderPass = renderpass;
  renderPassBeginInfo.renderArea.extent.width = dim;
  renderPassBeginInfo.renderArea.extent.height = dim;
  renderPassBeginInfo.clearValueCount = 1;
  renderPassBeginInfo.pClearValues = clearValues;
  renderPassBeginInfo.framebuffer = offscreen_framebuffer;

  std::vector<glm::mat4> matrices = {
    // POSITIVE_X
    glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    // NEGATIVE_X
    glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    // POSITIVE_Y
    glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    // NEGATIVE_Y
    glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    // POSITIVE_Z
    glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    // NEGATIVE_Z
    glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
  };

  cmd_buffer = dev::StaticHelpers::beginSingleTimeCommands(context_);
  VkViewport viewport{};
  viewport.width = (float)dim;
  viewport.height = (float)dim;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.extent.width = dim;
  scissor.extent.height = dim;
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
  vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

  subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount = numMips;
  subresourceRange.layerCount = 6;

  resources_->irradianceCube.setImageLayout(cmd_buffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
  
  for (uint32_t m = 0; m < numMips; m++) {
    for (uint32_t f = 0; f < 6; f++) {
      viewport.width = static_cast<float>(dim * std::pow(0.5f, m));
      viewport.height = static_cast<float>(dim * std::pow(0.5f, m));
      vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

      // Render scene from cube face's point of view
      vkCmdBeginRenderPass(cmd_buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

      // Update shader push constant block
      pushBlock.mvp = glm::perspective((float)(PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];

      vkCmdPushConstants(cmd_buffer, pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlock), &pushBlock);

      vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
      vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout, 0, 1, &descriptorset, 0, NULL);


      ///////////////////DRAW SKYBOX///////////////////
      Resources* intResources = ResourceManager::Get()->getResources();
      VkDeviceSize offsets[] = { 0 };
      VkBuffer vertexBuffers[] = { intResources->vertexBuffer.buffer_ };
      vkCmdBindVertexBuffers(cmd_buffer, 0, 1, vertexBuffers, offsets);
      vkCmdBindIndexBuffer(cmd_buffer, intResources->indicesBuffer.buffer_, 0, VK_INDEX_TYPE_UINT32);
      InternalVertexData vertex_data = intResources->vertex_data[(int)PrimitiveType::kPrimitiveType_Cube];
      uint32 first_vertex = vertex_data.offset;
      uint32 first_index = vertex_data.index_offset;
      vkCmdDrawIndexed(cmd_buffer, static_cast<uint32>(vertex_data.indices.size()), 1, first_index, first_vertex, 0);
      //models.skybox.draw(cmdBuf);

      vkCmdEndRenderPass(cmd_buffer);

      //subresourceRange = {};
      subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      subresourceRange.baseMipLevel = 0;
      subresourceRange.levelCount = 1;
      subresourceRange.layerCount = 1;
      offscreentexture.setImageLayout(cmd_buffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresourceRange);

      // Copy region for transfer from framebuffer to cube face
      VkImageCopy copyRegion = {};

      copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      copyRegion.srcSubresource.baseArrayLayer = 0;
      copyRegion.srcSubresource.mipLevel = 0;
      copyRegion.srcSubresource.layerCount = 1;
      copyRegion.srcOffset = { 0, 0, 0 };

      copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      copyRegion.dstSubresource.baseArrayLayer = f;
      copyRegion.dstSubresource.mipLevel = m;
      copyRegion.dstSubresource.layerCount = 1;
      copyRegion.dstOffset = { 0, 0, 0 };

      copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
      copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
      copyRegion.extent.depth = 1;

      vkCmdCopyImage(
        cmd_buffer,
        offscreentexture.image_,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        resources_->irradianceCube.image_,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &copyRegion);

      // Transform framebuffer color attachment back
      offscreentexture.setImageLayout(cmd_buffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, subresourceRange);
    }
  }

  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount = numMips;
  subresourceRange.layerCount = 6;
  resources_->irradianceCube.setImageLayout(cmd_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);

  dev::StaticHelpers::endSingleTimeCommands(context_, cmd_buffer);

  VkDevice device = context_->logDevice_;
  vkDestroyRenderPass(device, renderpass, nullptr);
  vkDestroyFramebuffer(device, offscreen_framebuffer, nullptr);
  offscreentexture.destroyTexture();
  vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
  vkDestroyDescriptorSetLayout(device, descriptorsetlayout, nullptr);
  vkDestroyPipeline(device, pipeline, nullptr);
  vkDestroyPipelineLayout(device, pipelinelayout, nullptr);
}


void VulkanApp::generatePrefilteredCube()
{
  const VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
  const int32_t dim = 512;
  const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;

  vkdev::VkTexture* prefilteredCube = &resources_->prefilteredCube;
  prefilteredCube->device_ = context_->logDevice_;
  prefilteredCube->width_ = dim;
  prefilteredCube->height_ = dim;
  prefilteredCube->mipLevels_ = numMips;
  prefilteredCube->createImage(context_->physDevice_, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
  //VkComponentMapping comp = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
  prefilteredCube->view_ = dev::StaticHelpers::createTextureImageView(context_->logDevice_, prefilteredCube->image_, format, 
                                                                      VK_IMAGE_VIEW_TYPE_CUBE, numMips, 6, VK_IMAGE_ASPECT_COLOR_BIT);
  prefilteredCube->sampler_ = dev::StaticHelpers::createTextureSampler(context_, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_COMPARE_OP_NEVER, 
                                                                       numMips, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, VK_FALSE);

  prefilteredCube->descriptor_ = {};
  prefilteredCube->descriptor_.imageView = prefilteredCube->view_;
  prefilteredCube->descriptor_.sampler = prefilteredCube->sampler_;
  prefilteredCube->descriptor_.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  // FB, Att, RP, Pipe, etc.
  VkAttachmentDescription attDesc = {};
  // Color attachment
  attDesc.format = format;
  attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
  attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

  VkSubpassDescription subpassDescription = {};
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pColorAttachments = &colorReference;

  // Use subpass dependencies for layout transitions
  std::array<VkSubpassDependency, 2> dependencies;
  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo renderPassCI{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  renderPassCI.attachmentCount = 1;
  renderPassCI.pAttachments = &attDesc;
  renderPassCI.subpassCount = 1;
  renderPassCI.pSubpasses = &subpassDescription;
  renderPassCI.dependencyCount = 2;
  renderPassCI.pDependencies = dependencies.data();

  VkRenderPass renderpass;
  vkCreateRenderPass(context_->logDevice_, &renderPassCI, nullptr, &renderpass);

  vkdev::VkTexture offscreentexture;
  VkFramebuffer offscreen_framebuffer;

  // Offfscreen framebuffer
  {
    // Color attachment
    //offscreentexture.alloc();
    offscreentexture.width_ = dim;
    offscreentexture.height_ = dim;
    offscreentexture.mipLevels_ = 1;
    offscreentexture.device_ = context_->logDevice_;
    offscreentexture.createImage(context_->physDevice_, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 1, 0);
    offscreentexture.view_ = dev::StaticHelpers::createTextureImageView(context_->logDevice_, offscreentexture.image_, format, VK_IMAGE_VIEW_TYPE_2D, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT);


    VkFramebufferCreateInfo fbufCreateInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    fbufCreateInfo.renderPass = renderpass;
    fbufCreateInfo.attachmentCount = 1;
    fbufCreateInfo.pAttachments = &offscreentexture.view_;
    fbufCreateInfo.width = dim;
    fbufCreateInfo.height = dim;
    fbufCreateInfo.layers = 1;
    vkCreateFramebuffer(context_->logDevice_, &fbufCreateInfo, nullptr, &offscreen_framebuffer);

    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;
    VkCommandBuffer cmd_buffer = dev::StaticHelpers::beginSingleTimeCommands(context_);
    offscreentexture.setImageLayout(cmd_buffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, subresourceRange);
    dev::StaticHelpers::endSingleTimeCommands(context_, cmd_buffer);
  }

  VkDescriptorSetLayout descriptorsetlayout;
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
    dev::StaticHelpers::layoutBindingInitializer(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)
  };

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = dev::StaticHelpers::setLayoutCreateInfoInitializer(setLayoutBindings);
  vkCreateDescriptorSetLayout(context_->logDevice_, &descriptorSetLayoutCI, nullptr, &descriptorsetlayout);

  // Descriptor Pool
  std::vector<VkDescriptorPoolSize> pool_sizes{ {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1} };

  VkDescriptorPoolCreateInfo descriptorPoolCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
  descriptorPoolCI.pPoolSizes = pool_sizes.data();
  descriptorPoolCI.poolSizeCount = static_cast<uint32>(pool_sizes.size());
  descriptorPoolCI.maxSets = 2;
  VkDescriptorPool descriptor_pool;
  vkCreateDescriptorPool(context_->logDevice_, &descriptorPoolCI, nullptr, &descriptor_pool);

  VkDescriptorSet descriptorset;
  VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
  allocInfo.descriptorPool = descriptor_pool;
  allocInfo.pSetLayouts = &descriptorsetlayout;
  allocInfo.descriptorSetCount = 1;
  vkAllocateDescriptorSets(context_->logDevice_, &allocInfo, &descriptorset);

  VkWriteDescriptorSet writeDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
  writeDescriptorSet.dstSet = descriptorset;
  writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  writeDescriptorSet.dstBinding = 0;
  writeDescriptorSet.descriptorCount = 1;
  ///////////////////////////////UPDATE IMAGE INFO/////////////////////////////////////
  uint32 skybox = resources_->internalMaterials[(uint32)MaterialType::kMaterialType_Skybox].texturesReferenced[0];
  writeDescriptorSet.pImageInfo = &resources_->itextures[skybox].descriptor_;

  vkUpdateDescriptorSets(context_->logDevice_, 1, &writeDescriptorSet, 0, nullptr);

  // Pipeline layout
  struct PushBlock {
    glm::mat4 mvp;
    // Sampling deltas
    float roughness;
    uint32 samples_number = 32u;
  } pushBlock;

  VkPipelineLayout pipelinelayout;
  std::vector<VkPushConstantRange> pushConstantRanges = {
    {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlock)},
  };

  VkPipelineLayoutCreateInfo pipelineLayoutCI{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
  pipelineLayoutCI.setLayoutCount = 1;
  pipelineLayoutCI.pSetLayouts = &descriptorsetlayout;
  pipelineLayoutCI.pushConstantRangeCount = 1;
  pipelineLayoutCI.pPushConstantRanges = pushConstantRanges.data();
  vkCreatePipelineLayout(context_->logDevice_, &pipelineLayoutCI, nullptr, &pipelinelayout);

  // Pipeline
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
  inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssemblyState.primitiveRestartEnable = VK_FALSE;

  VkPipelineRasterizationStateCreateInfo rasterizationState{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
  rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationState.cullMode = VK_CULL_MODE_NONE;
  rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizationState.depthClampEnable = VK_FALSE;
  rasterizationState.lineWidth = 1.0f;

  VkPipelineColorBlendAttachmentState blendAttachmentState{};
  blendAttachmentState.colorWriteMask = 0xf;
  blendAttachmentState.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlendState{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
  colorBlendState.attachmentCount = 1;
  colorBlendState.pAttachments = &blendAttachmentState;

  VkPipelineDepthStencilStateCreateInfo depthStencilState{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
  depthStencilState.depthTestEnable = VK_FALSE;
  depthStencilState.depthWriteEnable = VK_FALSE;
  depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

  VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineMultisampleStateCreateInfo multisampleState{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
  multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
  VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
  dynamicState.pDynamicStates = dynamicStateEnables.data();
  dynamicState.dynamicStateCount = static_cast<uint32>(dynamicStateEnables.size());

  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

  VkGraphicsPipelineCreateInfo pipelineCI{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
  pipelineCI.layout = pipelinelayout;
  pipelineCI.renderPass = renderpass;
  pipelineCI.basePipelineIndex = -1;
  pipelineCI.basePipelineHandle = VK_NULL_HANDLE;

  pipelineCI.pInputAssemblyState = &inputAssemblyState;
  pipelineCI.pRasterizationState = &rasterizationState;
  pipelineCI.pColorBlendState = &colorBlendState;
  pipelineCI.pMultisampleState = &multisampleState;
  pipelineCI.pViewportState = &viewportState;
  pipelineCI.pDepthStencilState = &depthStencilState;
  pipelineCI.pDynamicState = &dynamicState;
  pipelineCI.stageCount = 2;
  pipelineCI.pStages = shaderStages.data();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.pVertexBindingDescriptions = &InternalVertexData::getBindingDescription();
  std::vector<VkVertexInputAttributeDescription> attributeDescription = InternalVertexData::getAttributeDescription(VertexDescriptor::kVertexDescriptor_Pos);
  vertexInputInfo.vertexAttributeDescriptionCount = attributeDescription.size();
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

  pipelineCI.pVertexInputState = &vertexInputInfo;

  auto vertex_shader = dev::StaticHelpers::loadShader("./../../src/shaders/spir-v/filtercube_vert.spv");
  auto fragment_shader = dev::StaticHelpers::loadShader("./../../src/shaders/spir-v/prefilterenvmap_frag.spv");

  VkShaderModule vert_module = dev::StaticHelpers::createShaderModule(context_->logDevice_, vertex_shader);
  VkShaderModule frag_module = dev::StaticHelpers::createShaderModule(context_->logDevice_, fragment_shader);

  shaderStages[0] = {};
  shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStages[0].module = vert_module;
  shaderStages[0].pName = "main";

  shaderStages[1] = {};
  shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStages[1].module = frag_module;
  shaderStages[1].pName = "main";

  VkPipeline pipeline;
  vkCreateGraphicsPipelines(context_->logDevice_, resources_->pipelineCache, 1, &pipelineCI, nullptr, &pipeline);

  vkDestroyShaderModule(context_->logDevice_, vert_module, nullptr);
  vkDestroyShaderModule(context_->logDevice_, frag_module, nullptr);

  VkClearValue clearValues[1];
  clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };

  VkRenderPassBeginInfo renderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
  renderPassBeginInfo.renderPass = renderpass;
  renderPassBeginInfo.renderArea.extent.width = dim;
  renderPassBeginInfo.renderArea.extent.height = dim;
  renderPassBeginInfo.clearValueCount = 1;
  renderPassBeginInfo.pClearValues = clearValues;
  renderPassBeginInfo.framebuffer = offscreen_framebuffer;

  std::vector<glm::mat4> matrices = {
    // POSITIVE_X
    glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    // NEGATIVE_X
    glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    // POSITIVE_Y
    glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    // NEGATIVE_Y
    glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    // POSITIVE_Z
    glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    // NEGATIVE_Z
    glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
  };

  VkCommandBuffer cmd_buffer = dev::StaticHelpers::beginSingleTimeCommands(context_);
  VkViewport viewport{};
  viewport.width = (float)dim;
  viewport.height = (float)dim;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.extent.width = dim;
  scissor.extent.height = dim;
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
  vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

  VkImageSubresourceRange subresourceRange{};
  subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount = numMips;
  subresourceRange.layerCount = 6;

  resources_->prefilteredCube.setImageLayout(cmd_buffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

  for (uint32_t m = 0; m < numMips; m++) {
    pushBlock.roughness = (float)m / (float)(numMips - 1);
    for (uint32_t f = 0; f < 6; f++) {
      viewport.width = static_cast<float>(dim * std::pow(0.5f, m));
      viewport.height = static_cast<float>(dim * std::pow(0.5f, m));
      vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

      // Render scene from cube face's point of view
      vkCmdBeginRenderPass(cmd_buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

      // Update shader push constant block
      pushBlock.mvp = glm::perspective((float)(PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];

      vkCmdPushConstants(cmd_buffer, pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlock), &pushBlock);

      vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
      vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout, 0, 1, &descriptorset, 0, NULL);

      ///////////////////DRAW SKYBOX///////////////////
      Resources* intResources = ResourceManager::Get()->getResources();
      VkDeviceSize offsets[] = { 0 };
      VkBuffer vertexBuffers[] = { intResources->vertexBuffer.buffer_ };
      vkCmdBindVertexBuffers(cmd_buffer, 0, 1, vertexBuffers, offsets);
      vkCmdBindIndexBuffer(cmd_buffer, intResources->indicesBuffer.buffer_, 0, VK_INDEX_TYPE_UINT32);
      InternalVertexData vertex_data = intResources->vertex_data[(uint32)PrimitiveType::kPrimitiveType_Cube];
      uint32 first_vertex = vertex_data.offset;
      uint32 first_index = vertex_data.index_offset;
      vkCmdDrawIndexed(cmd_buffer, static_cast<uint32>(vertex_data.indices.size()), 1, first_index, first_vertex, 0);

      vkCmdEndRenderPass(cmd_buffer);

      subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      subresourceRange.baseMipLevel = 0;
      subresourceRange.levelCount = 1;
      subresourceRange.layerCount = 1;
      offscreentexture.setImageLayout(cmd_buffer, 
                                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
                                      subresourceRange);

      VkImageCopy copyRegion = {};

      copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      copyRegion.srcSubresource.baseArrayLayer = 0;
      copyRegion.srcSubresource.mipLevel = 0;
      copyRegion.srcSubresource.layerCount = 1;
      copyRegion.srcOffset = { 0, 0, 0 };

      copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      copyRegion.dstSubresource.baseArrayLayer = f;
      copyRegion.dstSubresource.mipLevel = m;
      copyRegion.dstSubresource.layerCount = 1;
      copyRegion.dstOffset = { 0, 0, 0 };

      copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
      copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
      copyRegion.extent.depth = 1;

      vkCmdCopyImage(
        cmd_buffer,
        offscreentexture.image_,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        resources_->prefilteredCube.image_,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &copyRegion);

      offscreentexture.setImageLayout(cmd_buffer, 
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                      subresourceRange);

    }
  }

  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount = numMips;
  subresourceRange.layerCount = 6;
  resources_->prefilteredCube.setImageLayout(cmd_buffer, 
                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
                                             subresourceRange);

  dev::StaticHelpers::endSingleTimeCommands(context_, cmd_buffer);

  VkDevice device = context_->logDevice_;
  vkDestroyRenderPass(device, renderpass, nullptr);
  vkDestroyFramebuffer(device, offscreen_framebuffer, nullptr);
  offscreentexture.destroyTexture();
  vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
  vkDestroyDescriptorSetLayout(device, descriptorsetlayout, nullptr);
  vkDestroyPipeline(device, pipeline, nullptr);
  vkDestroyPipelineLayout(device, pipelinelayout, nullptr);
}

uint32 VulkanApp::generateIBLTextures()
{
  InternalMaterial* mat = &resources_->internalMaterials[(uint32)MaterialType::kMaterialType_Skybox];
  if (mat->texturesReferenced.empty()) {
    return 1;
  }

  generateBRDFLUT();
  generateIrradianceCube();
  generatePrefilteredCube();
  return 0;
}

void VulkanApp::generateNoiseTexture(uint32 width, uint32 height)
{

  vkdev::VkTexture* noisetext = &resources_->noiseTexture;
  noisetext->width_ = width;
  noisetext->height_ = height;
  noisetext->mipLevels_ = 1;
  noisetext->device_ = context_->logDevice_;

  noisetext->createImage(context_->physDevice_, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 0);
  noisetext->view_ = dev::StaticHelpers::createTextureImageView(context_->logDevice_, noisetext->image_, VK_FORMAT_R8_UNORM, VK_IMAGE_VIEW_TYPE_2D, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT);
  noisetext->sampler_ = dev::StaticHelpers::createTextureSampler(context_, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_COMPARE_OP_NEVER, 0, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, VK_FALSE);

  noisetext->descriptor_.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  noisetext->descriptor_.imageView = noisetext->view_;
  noisetext->descriptor_.sampler = noisetext->sampler_;

  updateNoiseTexture(noisetext);

  resources_->grassTerrainTexture.loadImage(context_, "./../../data/textures/grass.jpg", VK_FORMAT_R8G8B8A8_SRGB);
  resources_->rockTerrainTexture.loadImage(context_, "./../../data/textures/mountain_rock.jpg", VK_FORMAT_R8G8B8A8_SRGB);
}

void VulkanApp::updateNoiseTexture(vkdev::VkTexture* texture)
{
  int32 w = texture->width_;
  int32 h = texture->height_;
  const uint32 mem_size = w * h;

  uint8* data = new uint8[mem_size];
  memset(data, 0, mem_size);

  PerlinNoise perlin_noise;
  FractalNoise fractal(perlin_noise);
  const float scale = static_cast<float>(rand() % 10) + 4.0f;

  for (size_t i = 0; i < h; i++) {
    for (size_t j = 0; j < w; j++) {
      float nw = j / (float)w;
      float nh = i / (float)h;

      //float n = 20.0f * perlin_noise.noise(nw, nh, 0);
      float n = fractal.noise(nw * scale, nh * scale, 0);
      n = n - floor(n);
      data[i * w + j] = static_cast<uint8>(floor(n * 255));
    }
  }

  vkdev::Buffer image_buffer;
  VkDeviceSize size = image_buffer.createBuffer(context_, mem_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  uint8* mapped;
  vkMapMemory(context_->logDevice_, image_buffer.memory_, 0, size, 0, (void**)&mapped);
  memcpy(mapped, data, mem_size);
  vkUnmapMemory(context_->logDevice_, image_buffer.memory_);

  VkCommandBuffer cmd_buffer;
  cmd_buffer = dev::StaticHelpers::beginSingleTimeCommands(context_);

  VkImageSubresourceRange subresource_range{};
  subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresource_range.baseMipLevel = 0;
  subresource_range.levelCount = 1;
  subresource_range.layerCount = 1;

  texture->setImageLayout(cmd_buffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresource_range);

  VkBufferImageCopy copy_region{};
  copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  copy_region.imageSubresource.mipLevel = 0;
  copy_region.imageExtent.width = w;
  copy_region.imageExtent.height = h;
  copy_region.imageExtent.depth = 1;
  copy_region.imageSubresource.baseArrayLayer = 0;
  copy_region.imageSubresource.layerCount = 1;

  vkCmdCopyBufferToImage(cmd_buffer, image_buffer.buffer_, texture->image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

  texture->layout_ = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  texture->setImageLayout(cmd_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture->layout_, subresource_range);

  dev::StaticHelpers::endSingleTimeCommands(context_, cmd_buffer);
  delete[] data;
  image_buffer.destroyBuffer();
}

void VulkanApp::createDescriptorSetLayout()
{
  Resources* res = ResourceManager::Get()->getResources();

  std::vector<VkDescriptorSetLayoutBinding> layoutBinding(2);
  layoutBinding[0].binding = 0;
  layoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  layoutBinding[0].descriptorCount = 1;
  layoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  layoutBinding[0].pImmutableSamplers = nullptr;

  layoutBinding[1].binding = 1;
  layoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  layoutBinding[1].descriptorCount = 1;
  layoutBinding[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  layoutBinding[1].pImmutableSamplers = nullptr;


  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32>(layoutBinding.size());
  layoutInfo.pBindings = layoutBinding.data();

  assert(vkCreateDescriptorSetLayout(context_->logDevice_, &layoutInfo, nullptr,
    &res->layouts[kLayoutType_Simple_2Binds].descriptor) == VK_SUCCESS);


  layoutBinding.clear();
  layoutBinding.resize(3);
  layoutBinding[0].binding = 0;
  layoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  layoutBinding[0].descriptorCount = 1;
  layoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  layoutBinding[0].pImmutableSamplers = nullptr;

  layoutBinding[1].binding = 1;
  layoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  layoutBinding[1].descriptorCount = 1;
  layoutBinding[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  layoutBinding[1].pImmutableSamplers = nullptr;

  layoutBinding[2].binding = 2;
  layoutBinding[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  layoutBinding[2].descriptorCount = kTexturePerShader;
  layoutBinding[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  layoutBinding[2].pImmutableSamplers = nullptr;

  layoutInfo.bindingCount = static_cast<uint32>(layoutBinding.size());
  layoutInfo.pBindings = layoutBinding.data();

  assert(vkCreateDescriptorSetLayout(context_->logDevice_, &layoutInfo, nullptr,
    &res->layouts[kLayoutType_Texture_3Binds].descriptor) == VK_SUCCESS);


  layoutBinding[2].descriptorCount = 1;

  assert(vkCreateDescriptorSetLayout(context_->logDevice_, &layoutInfo, nullptr,
    &res->layouts[kLayoutType_Texture_Cubemap].descriptor) == VK_SUCCESS);


  layoutBinding.clear();
  layoutBinding.resize(5);
  layoutBinding[0].binding = 0;
  layoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  layoutBinding[0].descriptorCount = 1;
  layoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  layoutBinding[0].pImmutableSamplers = nullptr;

  layoutBinding[1].binding = 1;
  layoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  layoutBinding[1].descriptorCount = 1;
  layoutBinding[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  layoutBinding[1].pImmutableSamplers = nullptr;

  layoutBinding[2].binding = 2;
  layoutBinding[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  layoutBinding[2].descriptorCount = 1;
  layoutBinding[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  layoutBinding[2].pImmutableSamplers = nullptr;

  layoutBinding[3].binding = 3;
  layoutBinding[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  layoutBinding[3].descriptorCount = 1;
  layoutBinding[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  layoutBinding[3].pImmutableSamplers = nullptr;

  layoutBinding[4].binding = 4;
  layoutBinding[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  layoutBinding[4].descriptorCount = 1;
  layoutBinding[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  layoutBinding[4].pImmutableSamplers = nullptr;

  layoutInfo.bindingCount = static_cast<uint32>(layoutBinding.size());
  layoutInfo.pBindings = layoutBinding.data();

  assert(vkCreateDescriptorSetLayout(context_->logDevice_, &layoutInfo, nullptr,
    &res->layouts[kLayoutType_PBRIBL].descriptor) == VK_SUCCESS);

  layoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  layoutBinding[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  layoutBinding[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  assert(vkCreateDescriptorSetLayout(context_->logDevice_, &layoutInfo, nullptr,
    &res->layouts[kLayoutType_Noise].descriptor) == VK_SUCCESS);
}

/*********************************************************************************************/

void VulkanApp::createDescriptorPool()
{
  uint32 descriptor_size = static_cast<uint32>(context_->swapchainImageViews.size());

  for (size_t i = 0; i < (int32)MaterialType::kMaterialType_MAX; i++) {
    InternalMaterial* mat = &resources_->internalMaterials[i];
    std::vector<VkDescriptorPoolSize> poolSizes;
    switch (mat->layout) {
      case kLayoutType_Simple_2Binds: {
        poolSizes.resize(2);
        poolSizes[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , descriptor_size };
        poolSizes[1] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC , descriptor_size };
        break;
      }
      case kLayoutType_PBRIBL:
      case kLayoutType_Noise:
      case kLayoutType_Texture_3Binds:
      case kLayoutType_Texture_Cubemap: {
        poolSizes.resize(3);
        poolSizes[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , descriptor_size };
        poolSizes[1] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC , descriptor_size };
        poolSizes[2] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , descriptor_size };
        break;
      }
      //case kLayoutType_PBRIBL: {
      //  poolSizes.resize(3);
      //  poolSizes[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , descriptor_size };
      //  poolSizes[1] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC , descriptor_size };
      //  poolSizes[2] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , 3 * descriptor_size };
      //  break;
      //}
      default: {
        break;
      }
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    poolInfo.maxSets = descriptor_size;

#ifdef NDEBUG
    (vkCreateDescriptorPool(context_->logDevice_, &poolInfo, nullptr,
      &resources_->internalMaterials[i].matDesciptorPool);
#else
    assert(vkCreateDescriptorPool(context_->logDevice_, &poolInfo, nullptr,
      &resources_->internalMaterials[i].matDesciptorPool) == VK_SUCCESS);
#endif
  }
}

/*********************************************************************************************/

std::vector<VkWriteDescriptorSet> getSimpleLayoutBinding(VkDescriptorSet descset, 
                                              VkDescriptorBufferInfo* bufferdesc,
                                  std::vector<VkDescriptorImageInfo>* image_info = nullptr,
                                                            Resources* resources = nullptr) {

  std::vector<VkWriteDescriptorSet> descriptor_write;
  descriptor_write.resize(2);
  descriptor_write[0] = dev::StaticHelpers::descriptorWriteInitializer(0,
                                       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                 descset,
                                                         &bufferdesc[0]);

  descriptor_write[1] = dev::StaticHelpers::descriptorWriteInitializer(1,
                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                                                 descset,
                                                         &bufferdesc[1]);

  return descriptor_write;
}

std::vector<VkWriteDescriptorSet> getTextureLayoutBinding(VkDescriptorSet descset, 
                                               VkDescriptorBufferInfo* bufferdesc,
                                   std::vector<VkDescriptorImageInfo>* image_info,
                                                             Resources* resources = nullptr) {
  
  std::vector<VkWriteDescriptorSet> descriptor_write;
  descriptor_write.resize(3);
  descriptor_write[0] = dev::StaticHelpers::descriptorWriteInitializer(0,
                                       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                 descset,
                                                         &bufferdesc[0]);

  descriptor_write[1] = dev::StaticHelpers::descriptorWriteInitializer(1,
                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                                                 descset,
                                                         &bufferdesc[1]);

  descriptor_write[2] = dev::StaticHelpers::descriptorWriteInitializer(2,
                               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                                 descset,
                                                      image_info->data(),
                                                      image_info->size());

  return descriptor_write;
}

std::vector<VkWriteDescriptorSet> getIBLLayoutBinding(VkDescriptorSet descset, 
                                           VkDescriptorBufferInfo* bufferdesc,
                               std::vector<VkDescriptorImageInfo>* image_info = nullptr,
                                                         Resources* resources = nullptr) {
  
  std::vector<VkWriteDescriptorSet> descriptor_write;
  descriptor_write.resize(5);
  descriptor_write[0] = dev::StaticHelpers::descriptorWriteInitializer(0,
                                       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                 descset,
                                                         &bufferdesc[0]);

  descriptor_write[1] = dev::StaticHelpers::descriptorWriteInitializer(1,
                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                                                 descset,
                                                         &bufferdesc[1]);

  descriptor_write[2] = dev::StaticHelpers::descriptorWriteInitializer(2,
                               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                                 descset,
                                  &resources->irradianceCube.descriptor_);

  descriptor_write[3] = dev::StaticHelpers::descriptorWriteInitializer(3,
                               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                                 descset,
                                          &resources->brdf.descriptor_);

  descriptor_write[4] = dev::StaticHelpers::descriptorWriteInitializer(4,
                               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                                 descset,
                                &resources->prefilteredCube.descriptor_);

  return descriptor_write;
}

std::vector<VkWriteDescriptorSet> getNoiseLayoutBinding(VkDescriptorSet descset, 
                                             VkDescriptorBufferInfo* bufferdesc,
                                 std::vector<VkDescriptorImageInfo>* image_info = nullptr,
                                                           Resources* resources = nullptr) {
  
  std::vector<VkWriteDescriptorSet> descriptor_write;
  descriptor_write.resize(5);
  descriptor_write[0] = dev::StaticHelpers::descriptorWriteInitializer(0,
                                       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                 descset,
                                                         &bufferdesc[0]);

  descriptor_write[1] = dev::StaticHelpers::descriptorWriteInitializer(1,
                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                                                 descset,
                                                         &bufferdesc[1]);

  descriptor_write[2] = dev::StaticHelpers::descriptorWriteInitializer(2,
                               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                                 descset, 
                                   &resources->noiseTexture.descriptor_);

  descriptor_write[3] = dev::StaticHelpers::descriptorWriteInitializer(3,
                               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                                 descset,
                              &resources->rockTerrainTexture.descriptor_);

  descriptor_write[4] = dev::StaticHelpers::descriptorWriteInitializer(4,
                               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                                 descset,
                            &resources->grassTerrainTexture.descriptor_);

  return descriptor_write;
}

void VulkanApp::createDescriptorSets()
{
  Resources* resources = ResourceManager::Get()->getResources();

  std::vector<VkWriteDescriptorSet> (*f[kLayoutType_MAX])(VkDescriptorSet descset,
                                              VkDescriptorBufferInfo * bufferdesc,
                                    std::vector<VkDescriptorImageInfo>*image_info,
                                                            Resources * resources);

  f[kLayoutType_Simple_2Binds] = &getSimpleLayoutBinding;
  f[kLayoutType_Texture_3Binds] = &getTextureLayoutBinding;
  f[kLayoutType_Texture_Cubemap] = &getTextureLayoutBinding;
  f[kLayoutType_PBRIBL] = &getIBLLayoutBinding;
  f[kLayoutType_Noise] = &getNoiseLayoutBinding;

  for (size_t j = 0; j < (int32)MaterialType::kMaterialType_MAX; j++) {
    InternalMaterial* mat = &resources->internalMaterials[j];
    if (mat->entitiesReferenced) {
      std::vector<VkDescriptorSetLayout> layouts(context_->swapchainImageViews.size(), 
                                         resources->layouts[mat->layout].descriptor);
      VkDescriptorSetAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      allocInfo.descriptorPool = mat->matDesciptorPool;
      allocInfo.descriptorSetCount = static_cast<uint32>(context_->swapchainImageViews.size());
      allocInfo.pSetLayouts = layouts.data();
      mat->matDescriptorSet.resize(context_->swapchainImageViews.size());
      if (vkAllocateDescriptorSets(context_->logDevice_, &allocInfo,
        mat->matDescriptorSet.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets");
      }

      
      std::vector<VkDescriptorImageInfo> image_info;
      uint32 texture_number = mat->texturesReferenced.size();
      image_info.resize(texture_number);
      for (size_t i = 0; i < texture_number; i++) {
        VkDescriptorImageInfo img_info{};
        img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkdev::VkTexture* tex = &resources->itextures[mat->texturesReferenced[i]];
        img_info.imageView = tex->view_;
        img_info.sampler = tex->sampler_;
        image_info[i] = img_info;
      }

        VkDescriptorBufferInfo bufferSceneInfo{};
        bufferSceneInfo.offset = 0;
        bufferSceneInfo.range = sizeof(SceneUniformBuffer);
        VkDescriptorBufferInfo bufferObjectInfo{};
        bufferObjectInfo.offset = 0;
        bufferObjectInfo.range = sizeof(UniformBlocks);
        VkDescriptorBufferInfo buffer_descriptor[] = { bufferSceneInfo, bufferObjectInfo };
      for (size_t i = 0; i < context_->swapchainImageViews.size(); i++) {
        buffer_descriptor[0].buffer = resources->staticUniform[i].buffer_;
        buffer_descriptor[1].buffer = mat->dynamicUniform[i].buffer_;

        std::vector<VkWriteDescriptorSet> descriptor_write = f[mat->layout](mat->matDescriptorSet[i],
                                                                                   buffer_descriptor,
                                                                                         &image_info,
                                                                                         resources_);

        vkUpdateDescriptorSets(context_->logDevice_, descriptor_write.size(), descriptor_write.data(), 0, nullptr);
      }
    }
  }
}

/*********************************************************************************************/

void VulkanApp::createUniformBuffers()
{
  uint64_t dynamicAlignment = dev::StaticHelpers::padUniformBufferOffset(context_, sizeof(UniformBlocks));

  Resources* resources = ResourceManager::Get()->getResources();
  uint32 swapChainImageCount = context_->swapchainImageViews.size();
  for (size_t j = 0; j < (int32)MaterialType::kMaterialType_MAX; j++) {
    InternalMaterial* mat = &resources->internalMaterials[j];
    mat->dynamicUniform.resize(swapChainImageCount);
    if (mat->entitiesReferenced) {
    VkDeviceSize dynamicBufferSize = dynamicAlignment * mat->entitiesReferenced;
      mat->dynamicUniformData = (UniformBlocks*)_aligned_malloc(dynamicBufferSize, dynamicAlignment);
      for (size_t i = 0; i < swapChainImageCount; i++) {
        mat->dynamicUniform[i].createBuffer(context_, dynamicBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        vkMapMemory(context_->logDevice_, mat->dynamicUniform[i].memory_, 0, dynamicBufferSize, 0, &mat->dynamicUniform[i].mapped_);
      }
    }
  }

  resources->staticUniform.resize(swapChainImageCount);
  for (size_t i = 0; i < swapChainImageCount; i++) {
    resources->staticUniform[i].createBuffer(context_, sizeof(SceneUniformBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkMapMemory(context_->logDevice_, resources->staticUniform[i].memory_, 0, 
                sizeof(SceneUniformBuffer), 0, &resources->staticUniform[i].mapped_);
  }
}

/*********************************************************************************************/

void VulkanApp::updateUniformBuffers(uint32 index)
{
  Resources* resources = ResourceManager::Get()->getResources();

  UpdateData update_data{};
  update_data.sceneBuffer.view = Scene::camera.getView();
  update_data.sceneBuffer.projection = Scene::camera.getProjection();
  update_data.sceneBuffer.cameraPosition = { Scene::camera.getPosition() };
  update_data.sceneBuffer.lightNumber = 0;
  Resources* res = ResourceManager::Get()->getResources();
  uint64_t padding = dev::StaticHelpers::padUniformBufferOffset(context_, sizeof(UniformBlocks));
  for (size_t i = 0; i < Scene::entitiesCount; i++) {
    Entity* entity = Scene::sceneEntities[i].get();
    update_data.drawCall.geometry = -1;
    entity->updateEntity(&update_data, padding);
    if (update_data.drawCall.geometry > -1) res->draw_calls.push(update_data.drawCall);
  }

  memcpy(resources->staticUniform[index].mapped_, &update_data.sceneBuffer, sizeof(SceneUniformBuffer));

  for (auto& internal_material : resources->internalMaterials) {
    uint64_t sceneUboSize = internal_material.entitiesReferenced * padding;
    memcpy(internal_material.dynamicUniform[index].mapped_, internal_material.dynamicUniformData, sceneUboSize);
  }
}

/*********************************************************************************************/

void VulkanApp::initFrameData(uint32 frame_count)
{
  for (size_t i = 0; i < frame_count; i++) {
    VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    //assert(vkCreateFence(context_->logDevice_, &fenceInfo, nullptr, &context_->perFrame[i].submitFence) == VK_SUCCESS);
    vkCreateFence(context_->logDevice_, &fenceInfo, nullptr, &context_->perFrame[i].submitFence);

    VkCommandPoolCreateInfo commandPoolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    QueueFamilyIndices graphicIndices = dev::StaticHelpers::findQueueFamilies(context_->physDevice_, context_->surface);
    commandPoolInfo.queueFamilyIndex = graphicIndices.graphicsFamily;
    assert(vkCreateCommandPool(context_->logDevice_, &commandPoolInfo, nullptr, 
                               &context_->perFrame[i].primaryCommandPool) == VK_SUCCESS);

    VkCommandBufferAllocateInfo cmdBufferInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cmdBufferInfo.commandPool = context_->perFrame[i].primaryCommandPool;
    cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferInfo.commandBufferCount = 1;
    assert(vkAllocateCommandBuffers(context_->logDevice_, &cmdBufferInfo, 
                                    &context_->perFrame[i].primaryCommandBuffer) == VK_SUCCESS);
  }
}

/*********************************************************************************************/

void VulkanApp::destroyFrameData(FrameData& frame_data)
{
  if (frame_data.submitFence != VK_NULL_HANDLE) {
    vkDestroyFence(context_->logDevice_, frame_data.submitFence, nullptr);
    frame_data.submitFence = VK_NULL_HANDLE;
  }

  if (frame_data.primaryCommandBuffer != VK_NULL_HANDLE) {
    vkFreeCommandBuffers(context_->logDevice_, frame_data.primaryCommandPool, 
                                        1, &frame_data.primaryCommandBuffer);
    frame_data.primaryCommandBuffer = VK_NULL_HANDLE;
  }

  if (frame_data.primaryCommandPool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(context_->logDevice_, frame_data.primaryCommandPool, nullptr);
    frame_data.primaryCommandPool = VK_NULL_HANDLE;
  }

  if (frame_data.swapchainAcquire != VK_NULL_HANDLE) {
    vkDestroySemaphore(context_->logDevice_, frame_data.swapchainAcquire, nullptr);
    frame_data.swapchainAcquire = VK_NULL_HANDLE;
  }

  if (frame_data.swapchainRelease != VK_NULL_HANDLE) {
    vkDestroySemaphore(context_->logDevice_, frame_data.swapchainRelease, nullptr);
    frame_data.swapchainRelease = VK_NULL_HANDLE;
  }
}

/*********************************************************************************************/

int32 VulkanApp::acquireNextImage(uint32* image)
{
  VkSemaphore acquireSemaphore;
  if (context_->recycledSemaphores.empty()) {
    VkSemaphoreCreateInfo info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    vkCreateSemaphore(context_->logDevice_, &info, nullptr, &acquireSemaphore);
  }
  else {
    acquireSemaphore = context_->recycledSemaphores.back();
    context_->recycledSemaphores.pop_back();
  }

  //Getting the next image in the swapchain and their index
  VkResult res = vkAcquireNextImageKHR(context_->logDevice_, context_->swapChain, 
                                       UINT64_MAX, acquireSemaphore, VK_NULL_HANDLE, image);

  /*If we can't get the image from the swapchain 
  we put the semaphore in the array and end the function*/
  if (res != VK_SUCCESS) {
    context_->recycledSemaphores.push_back(acquireSemaphore);
    return -1;
  }

  /*Wait for the fence if it isn't null in the current frame.
  After that we reset the fence*/
  if (context_->perFrame[*image].submitFence != VK_NULL_HANDLE) {
    vkWaitForFences(context_->logDevice_, 1, &context_->perFrame[*image].submitFence, true, UINT64_MAX);
    vkResetFences(context_->logDevice_, 1, &context_->perFrame[*image].submitFence);
  }

  /*Reset the command pool if isn't null*/
  if (context_->perFrame[*image].primaryCommandPool != VK_NULL_HANDLE) {
    vkResetCommandPool(context_->logDevice_, context_->perFrame[*image].primaryCommandPool, 0);
  }
  
  /*Recycling the old swap chain semaphore*/
  VkSemaphore oldSemaphore = context_->perFrame[*image].swapchainAcquire;
  if (oldSemaphore != VK_NULL_HANDLE) {
    context_->recycledSemaphores.push_back(oldSemaphore);
  }

  context_->perFrame[*image].swapchainAcquire = acquireSemaphore;

  return 0;
}

/*********************************************************************************************/

void VulkanApp::render(uint32 index)
{
  VkFramebuffer framebuffer = context_->swapchainFramebuffers[index];

  VkCommandBuffer cmd_buffer = context_->perFrame[index].primaryCommandBuffer;

  VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(cmd_buffer, &begin_info);
  
  std::array<VkClearValue, 2> clearColor{};
  clearColor[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
  clearColor[1].depthStencil = { 1.0f, 0 };

  VkRenderPassBeginInfo rp_begin{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
  rp_begin.renderPass = context_->renderPass;
  rp_begin.framebuffer = framebuffer;
  rp_begin.renderArea.extent.width = context_->swapchainDimensions.width;
  rp_begin.renderArea.extent.height = context_->swapchainDimensions.height;
  rp_begin.clearValueCount = clearColor.size();
  rp_begin.pClearValues = clearColor.data();

  vkCmdBeginRenderPass(cmd_buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

  int64_t padding = dev::StaticHelpers::padUniformBufferOffset(context_, sizeof(UniformBlocks));

  std::queue<DrawCallData>* drawcs = &resources_->draw_calls;
  DrawCmd drawcmd;
  while (!drawcs->empty()) {
    drawcmd.Execute(cmd_buffer, drawcs->front(), index, padding);
    drawcs->pop();
  }

  vkCmdEndRenderPass(cmd_buffer);
  vkEndCommandBuffer(cmd_buffer);

  VkPipelineStageFlags waitStage{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

  VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmd_buffer;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &context_->perFrame[index].swapchainAcquire;
  submitInfo.pWaitDstStageMask = &waitStage;

  vkQueueSubmit(context_->graphicsQueue, 1, &submitInfo, context_->perFrame[index].submitFence);

}

/*********************************************************************************************/

int32 VulkanApp::presentImage(uint32 index)
{
  VkPresentInfoKHR present{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
  present.swapchainCount = 1;
  present.pSwapchains = &context_->swapChain;
  present.pImageIndices = &index;
  present.swapchainCount = 1;

  int32 result = vkQueuePresentKHR(context_->presentQueue, &present);

  return result;
}

/*********************************************************************************************/

void VulkanApp::drawFrame()
{
  uint32 imageIndex;

  auto result = acquireNextImage(&imageIndex);
  if (result) {
    vkQueueWaitIdle(context_->graphicsQueue);
    return;
  }

  updateUniformBuffers(imageIndex);

  render(imageIndex);
  result = presentImage(imageIndex);
}

/*********************************************************************************************/

VulkanApp::VulkanApp()
{
  context_ = new Context();
  debug_data_ = new DebugUtils();
  user_app_ = new UserMain();
  context_->physDevice_ = VK_NULL_HANDLE;
  resources_ = ResourceManager::Get()->getResources();
  ResourceManager::Get()->initPrimitiveGeometries();
}

/*********************************************************************************************/

VulkanApp::~VulkanApp()
{
  delete(debug_data_);
  delete(context_);
  delete(user_app_);
}

/*********************************************************************************************/

void VulkanApp::start()
{
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  context_->window_ = glfwCreateWindow(k_wWidth, k_wHeight, "TechnicalComputingDemo", nullptr, nullptr);
  glfwSetKeyCallback(context_->window_, InputManager::keyCallback);
  glfwSetInputMode(context_->window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(context_->window_, InputManager::mouseCallback);
  user_app_->init();
  createAppInstance();
  setupDebugMessenger();
  createSurface();
  setupPhysicalDevice();
  createLogicalDevice();
  createSwapChain();
  createRenderPass();
  createDescriptorSetLayout();
  createPipelineLayout();
  createPipelineCache();
  createInternalMaterials();
  createCommandPool();
  createDepthResource();
  createFramebuffer();
  storeTextures();
  createVertexBuffers();
  createIndexBuffers();
  generateNoiseTexture(512, 512);
  generateIBLTextures();

  createUniformBuffers();
  createDescriptorPool();
  createDescriptorSets();
}

/*********************************************************************************************/

void VulkanApp::loop()
{ 
  Scene::lastTime = std::chrono::high_resolution_clock::now();
  bool should_close = false;
  while (!should_close && !glfwWindowShouldClose(context_->window_)) {
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>
                                  (currentTime - Scene::lastTime).count();
    glfwPollEvents();
    Scene::camera.cameraInput(deltaTime);
    should_close = InputManager::getInputState(kKeyCode_ESC);
    user_app_->run(deltaTime);
    Scene::camera.updateCamera();
    drawFrame();

    Scene::lastTime = currentTime;
  }

  vkDeviceWaitIdle(context_->logDevice_);
}

/*********************************************************************************************/

void VulkanApp::end()
{
  user_app_->clear();

  vkQueueWaitIdle(context_->graphicsQueue);
  for (auto& framebuffer : context_->swapchainFramebuffers) {
    vkDestroyFramebuffer(context_->logDevice_, framebuffer, nullptr);
  }
  context_->swapchainFramebuffers.clear();

  for (auto& frame_data : context_->perFrame) {
    destroyFrameData(frame_data);
  }
  for (auto& semaphore : context_->recycledSemaphores) {
    vkDestroySemaphore(context_->logDevice_, semaphore, nullptr);
  }

  context_->perFrame.clear();
  vkDestroyCommandPool(context_->logDevice_, context_->transferCommandPool, nullptr);

  for (auto& layout : resources_->layouts) {
    vkDestroyPipelineLayout(context_->logDevice_, layout.pipeline, nullptr);
    vkDestroyDescriptorSetLayout(context_->logDevice_, layout.descriptor, nullptr);
  }

  vkDestroyPipelineCache(context_->logDevice_, resources_->pipelineCache, nullptr);

  for (auto& material : resources_->internalMaterials) {
    dev::StaticHelpers::destroyMaterial(context_, &material);
  }


  vkDestroyRenderPass(context_->logDevice_, context_->renderPass, nullptr);

  //Swap chain
  resources_->depthAttachment.destroyTexture();
  for (auto image_view : context_->swapchainImageViews) {
    vkDestroyImageView(context_->logDevice_, image_view, nullptr);
  }
  vkDestroySwapchainKHR(context_->logDevice_, context_->swapChain, nullptr);

  //Textures
  for (auto& texture : resources_->itextures) {
    texture.destroyTexture();
  }

  resources_->brdf.destroyTexture();
  resources_->irradianceCube.destroyTexture();
  resources_->prefilteredCube.destroyTexture();

  //Vertex Buffers
  ResourceManager* rm = ResourceManager::Get();
  resources_->vertexBuffer.destroyBuffer();
  resources_->indicesBuffer.destroyBuffer();
  delete(rm);
  

  vkDestroySurfaceKHR(context_->instance_, context_->surface, nullptr);
  vkDestroyDevice(context_->logDevice_, nullptr);
  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(context_->instance_, debug_data_->debugMessenger_, nullptr);
  }
  vkDestroyInstance(context_->instance_, nullptr);

  glfwDestroyWindow(context_->window_);
  glfwTerminate();
}

/*********************************************************************************************/
