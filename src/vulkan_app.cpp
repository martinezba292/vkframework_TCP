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
  //assert(vkCreateInstance(&createInfo, nullptr, &context_->instance_) == VK_SUCCESS);

  /** ENUMERATE VULKAN EXTENSIONS SUPPORT */
  /*
  uint32 extensionCount;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

  for (auto& extension : extensions) {
    printf("%s\n", extension.extensionName);
  }
  */
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

  /*if (!swapChainSupported) {
    return 0;
  }*/

  return score;
}

/************************************************************************************************/

void VulkanApp::setupPhysicalDevice()
{
  uint32 deviceCount = 0;
  vkEnumeratePhysicalDevices(context_->instance_, &deviceCount, nullptr);
  assert(0 < deviceCount);

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(context_->instance_, &deviceCount, devices.data());

  std::multimap<int32, VkPhysicalDevice> candidates;
  for (const auto& device : devices) {
    int32 score = rateDeviceSuitability(device, *context_);
    candidates.insert(std::make_pair(score, device));
  }

  VkPhysicalDeviceFeatures feat;
  if (candidates.rbegin()->first > 0) {
    //vkGetPhysicalDeviceFeatures(candidates.rbegin()->second, &feat);
    //feat.samplerAnisotropy = VK_TRUE;
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
  
  //assert(vkCreateDevice(context_->physDevice_, &deviceInfo, nullptr, &context_->logDevice_) == VK_SUCCESS);
  vkCreateDevice(context_->physDevice_, &deviceInfo, nullptr, &context_->logDevice_) == VK_SUCCESS;

  vkGetDeviceQueue(context_->logDevice_, indices.graphicsFamily, 0, &context_->graphicsQueue);
  vkGetDeviceQueue(context_->logDevice_, indices.presentFamily, 0, &context_->presentQueue);
}


/*********************************WINDOW SURFACE*************************************/

void VulkanApp::createSurface()
{
#ifdef NDEBUG
  glfwCreateWindowSurface(context_->instance_, context_->window_, nullptr, &context_->surface);
#else
  assert(glfwCreateWindowSurface(context_->instance_, context_->window_, nullptr, &context_->surface) == VK_SUCCESS);
#endif
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
  swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //VK_IMAGE_USAGE_TRANSFER_DST_BIT for post-processing

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

#ifdef NDEBUG
  vkCreateSwapchainKHR(context_->logDevice_, &swapChainInfo, nullptr, &context_->swapChain);
#else
  assert(vkCreateSwapchainKHR(context_->logDevice_, &swapChainInfo, nullptr, &context_->swapChain) == VK_SUCCESS);
#endif

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

void VulkanApp::createInternalMaterials()
{
  //Resources* res = ResourceManager::Get()->getResources();
  InternalMaterial* material = &resources_->internalMaterials[(int32)MaterialType::kMaterialType_UnlitColor];
  material->layout = kLayoutType_Simple_2Binds;
  material->matPipeline = dev::StaticHelpers::createPipeline(context_, "./../../src/shaders/spir-v/unlit_color_vert.spv",
                                                             "./../../src/shaders/spir-v/unlit_color_frag.spv",
                                                             resources_->layouts[kLayoutType_Simple_2Binds].pipeline, VK_CULL_MODE_BACK_BIT, VK_TRUE);

  material = &resources_->internalMaterials[(int32)MaterialType::kMaterialType_BasicPBR];
  material->layout = kLayoutType_Simple_2Binds;
  material->matPipeline = dev::StaticHelpers::createPipeline(context_, 
                                                             "./../../src/shaders/spir-v/basic_pbr_vert.spv",
                                                             "./../../src/shaders/spir-v/basic_pbr_frag.spv",
                                                             resources_->layouts[kLayoutType_Simple_2Binds].pipeline, VK_CULL_MODE_BACK_BIT, VK_TRUE);

  material = &resources_->internalMaterials[(int32)MaterialType::kMaterialType_TextureSampler];
  material->layout = kLayoutType_Texture_3Binds;
  material->matPipeline = dev::StaticHelpers::createPipeline(context_, 
                                                             "./../../src/shaders/spir-v/texture_sampling_vert.spv",
                                                             "./../../src/shaders/spir-v/texture_sampling_frag.spv",
                                                             resources_->layouts[kLayoutType_Texture_3Binds].pipeline, VK_CULL_MODE_BACK_BIT, VK_TRUE);

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
  /*VkSubpassDependency subpassDependency{};
  subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependency.dstSubpass = 0;
  subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  subpassDependency.srcAccessMask = 0;
  subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;*/

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

    /// !!!!!!!!!!!!!!!!!!!!!! ///
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
  depth->view_ = dev::StaticHelpers::createTextureImageView(context_->logDevice_, depth->image_, depth_format, VK_IMAGE_VIEW_TYPE_2D, 1, 1, VK_IMAGE_ASPECT_DEPTH_BIT);
}

/*********************************************************************************************/

void VulkanApp::storeTextures()
{
  uint32 textures_number = Scene::textureCount;
  resources_->itextures.resize(textures_number);

  for (size_t i = 0; i < textures_number; i++) {
    Texture* user_texture = Scene::userTextures[i].get();
    //vkdev::VkTexture new_texture;
    //if (user_texture->getType() == TextureType::kTextureType_Cubemap)
    //  resources_->itextures[i].loadCubemapKtx(context_, user_texture->getPath().c_str(), VK_FORMAT_R8G8B8A8_UNORM);
    //else
      resources_->itextures[i].loadImage(context_, user_texture->getPath().c_str());

    //resources_->itextures[i] = new_texture;
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
  for (size_t i = 0; i < geometry_number; i++) {
    mainResources->vertex_data[i].offset = vertex_offset;
    vertex_offset += mainResources->vertex_data[i].vertex.size();

    sizes[i] = (static_cast<uint64_t>(sizeof(Vertex)) * 
                        mainResources->vertex_data[i].vertex.size());
    offset_bytes[i] = total_vertex_bytes;
    total_vertex_bytes += sizes[i];
  }
  mainResources->vertexBuffer.createBuffer(context_, total_vertex_bytes, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  for (size_t i = 0; i < geometry_number; i++) {
    InternalVertexData* vertexData = &mainResources->vertex_data[i];

    VkDeviceSize size = sizes[i];
    vkdev::Buffer staging_buffer;
    staging_buffer.createBuffer(context_, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(context_->logDevice_, staging_buffer.memory_, 0, size, 0, &data);
    memcpy(data, vertexData->vertex.data(), size);
    vkUnmapMemory(context_->logDevice_, staging_buffer.memory_);

    mainResources->vertexBuffer.copyBuffer(context_, staging_buffer, size, offset_bytes[i]);
    //staging_buffer.destroyBuffer();
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

#ifdef NDEBUG
  (vkCreateDescriptorSetLayout(context_->logDevice_, &layoutInfo, nullptr,
                                  &res->layouts[kLayoutType_Simple_2Binds].descriptor);
#else
  assert(vkCreateDescriptorSetLayout(context_->logDevice_, &layoutInfo, nullptr,
    &res->layouts[kLayoutType_Simple_2Binds].descriptor) == VK_SUCCESS);
#endif

  std::vector<VkDescriptorSetLayoutBinding> layoutBinding3(3);
  layoutBinding3[0].binding = 0;
  layoutBinding3[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  layoutBinding3[0].descriptorCount = 1;
  layoutBinding3[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  layoutBinding3[0].pImmutableSamplers = nullptr;

  layoutBinding3[1].binding = 1;
  layoutBinding3[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  layoutBinding3[1].descriptorCount = 1;
  layoutBinding3[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  layoutBinding3[1].pImmutableSamplers = nullptr;

  layoutBinding3[2].binding = 2;
  layoutBinding3[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  layoutBinding3[2].descriptorCount = kTexturePerShader;
  layoutBinding3[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  layoutBinding3[2].pImmutableSamplers = nullptr;

  layoutInfo.bindingCount = static_cast<uint32>(layoutBinding3.size());
  layoutInfo.pBindings = layoutBinding3.data();

#ifdef NDEBUG
  (vkCreateDescriptorSetLayout(context_->logDevice_, &layoutInfo, nullptr,
    &res->layouts[kLayoutType_Texture_3Binds].descriptor);
#else
  assert(vkCreateDescriptorSetLayout(context_->logDevice_, &layoutInfo, nullptr,
    &res->layouts[kLayoutType_Texture_3Binds].descriptor) == VK_SUCCESS);
#endif

  std::vector<VkDescriptorSetLayoutBinding> layoutBindingCubemap(2);
  layoutBindingCubemap[0].binding = 0;
  layoutBindingCubemap[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  layoutBindingCubemap[0].descriptorCount = 1;
  layoutBindingCubemap[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  layoutBindingCubemap[0].pImmutableSamplers = nullptr;

  layoutBindingCubemap[1].binding = 1;
  layoutBindingCubemap[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  layoutBindingCubemap[1].descriptorCount = 1;
  layoutBindingCubemap[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  layoutBindingCubemap[1].pImmutableSamplers = nullptr;

  layoutInfo.bindingCount = static_cast<uint32>(layoutBindingCubemap.size());
  layoutInfo.pBindings = layoutBindingCubemap.data();

#ifdef NDEBUG
  (vkCreateDescriptorSetLayout(context_->logDevice_, &layoutInfo, nullptr,
    &res->layouts[kLayoutType_Texture_Cubemap].descriptor);
#else
  assert(vkCreateDescriptorSetLayout(context_->logDevice_, &layoutInfo, nullptr,
    &res->layouts[kLayoutType_Texture_Cubemap].descriptor) == VK_SUCCESS);
#endif
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
      case kLayoutType_Texture_3Binds: {
        poolSizes.resize(3);
        poolSizes[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , descriptor_size };
        poolSizes[1] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC , descriptor_size };
        poolSizes[2] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , descriptor_size };
        break;
      }
      case kLayoutType_Texture_Cubemap: {
        poolSizes.resize(2);
        poolSizes[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , descriptor_size };
        poolSizes[1] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , descriptor_size };
        break;
      }
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

void VulkanApp::createDescriptorSets()
{
  Resources* resources = ResourceManager::Get()->getResources();

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
      for (size_t i = 0; i < context_->swapchainImageViews.size(); i++) {
        bufferSceneInfo.buffer = resources->staticUniform[i].buffer_;
        bufferObjectInfo.buffer = mat->dynamicUniform[i].buffer_;
        std::vector<VkWriteDescriptorSet> descriptor_write;
        switch (mat->layout) {
          case kLayoutType_Simple_2Binds:{
            descriptor_write.resize(2);
            descriptor_write[0] = dev::StaticHelpers::descriptorWriteInitializer(0, 
                                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
                                                          mat->matDescriptorSet[i],
                                                                   &bufferSceneInfo);

            descriptor_write[1] = dev::StaticHelpers::descriptorWriteInitializer(1, 
                                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 
                                                          mat->matDescriptorSet[i],
                                                                  &bufferObjectInfo);
            break;
          }
          case kLayoutType_Texture_3Binds: {
            descriptor_write.resize(3);
            descriptor_write[0] = dev::StaticHelpers::descriptorWriteInitializer(0,
                                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                          mat->matDescriptorSet[i],
                                                                  &bufferSceneInfo);

            descriptor_write[1] = dev::StaticHelpers::descriptorWriteInitializer(1,
                                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                                          mat->matDescriptorSet[i],
                                                                 &bufferObjectInfo);

            descriptor_write[2] = dev::StaticHelpers::descriptorWriteInitializer(2,
                                         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                          mat->matDescriptorSet[i],
                                                                 image_info.data(),
                                                                 image_info.size());
            break;
          }
          case kLayoutType_Texture_Cubemap: {
            descriptor_write.resize(2);
            descriptor_write[0] = dev::StaticHelpers::descriptorWriteInitializer(0,
                                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                          mat->matDescriptorSet[i],
                                                                  &bufferSceneInfo);

            descriptor_write[1] = dev::StaticHelpers::descriptorWriteInitializer(1,
                                         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                          mat->matDescriptorSet[i],
                                                                 image_info.data(),
                                                                 image_info.size());
            break;
          }
          default:
            break;
        }

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

    vkMapMemory(context_->logDevice_, resources->staticUniform[i].memory_, 0, sizeof(SceneUniformBuffer), 0, &resources->staticUniform[i].mapped_);
  }
}

/*********************************************************************************************/

void VulkanApp::updateUniformBuffers(uint32 index)
{
  Resources* resources = ResourceManager::Get()->getResources();

  ComponentUpdateData update_data{};
  update_data.sceneBuffer.view = Scene::camera.getView();
  update_data.sceneBuffer.projection = Scene::camera.getProjection();
  update_data.sceneBuffer.cameraPosition = { Scene::camera.getPosition() };
  update_data.sceneBuffer.lightNumber = 0;
  Resources* res = ResourceManager::Get()->getResources();
  uint64_t padding = dev::StaticHelpers::padUniformBufferOffset(context_, sizeof(UniformBlocks));
  for (size_t i = 0; i < Scene::entitiesCount; i++) {
    Entity* entity = Scene::sceneEntities[i].get();
    Material* mat = entity->getMaterial();
    entity->updateComponents(&update_data);
    if (mat) {
      int32 offset = entity->getMaterialOffset();
      UniformBlocks* buffer = (UniformBlocks*)((uint64_t)resources->internalMaterials[mat->getMaterialType()].dynamicUniformData +
        (offset * padding));
      *buffer = update_data.objectBuffer;
    }
    if (update_data.drawCall.geometry) res->draw_calls.push_back(update_data.drawCall);
  }

  memcpy(resources->staticUniform[index].mapped_, &update_data.sceneBuffer, sizeof(SceneUniformBuffer));

  for (auto& internal_material : resources->internalMaterials) {
    uint64_t sceneUboSize = internal_material.entitiesReferenced * dev::StaticHelpers::padUniformBufferOffset(context_, sizeof(UniformBlocks));
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
    //assert(vkCreateCommandPool(context_->logDevice_, &commandPoolInfo, nullptr, &context_->perFrame[i].primaryCommandPool) == VK_SUCCESS);
    vkCreateCommandPool(context_->logDevice_, &commandPoolInfo, nullptr, &context_->perFrame[i].primaryCommandPool);

    VkCommandBufferAllocateInfo cmdBufferInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cmdBufferInfo.commandPool = context_->perFrame[i].primaryCommandPool;
    cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferInfo.commandBufferCount = 1;
    //assert(vkAllocateCommandBuffers(context_->logDevice_, &cmdBufferInfo, &context_->perFrame[i].primaryCommandBuffer) == VK_SUCCESS);
    vkAllocateCommandBuffers(context_->logDevice_, &cmdBufferInfo, &context_->perFrame[i].primaryCommandBuffer);

    context_->perFrame[i].device = context_->logDevice_;
    context_->perFrame[i].graphicsQueue = graphicIndices.graphicsFamily;
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

  frame_data.device = VK_NULL_HANDLE;
  frame_data.graphicsQueue = -1;
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
  VkResult res = vkAcquireNextImageKHR(context_->logDevice_, context_->swapChain, UINT64_MAX, acquireSemaphore, VK_NULL_HANDLE, image);

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
  clearColor[0].color = { 0.53f, 0.81f, 0.92f, 1.0f };
  clearColor[1].depthStencil = { 1.0f, 0 };

  VkRenderPassBeginInfo rp_begin{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
  rp_begin.renderPass = context_->renderPass;
  rp_begin.framebuffer = framebuffer;
  rp_begin.renderArea.extent.width = context_->swapchainDimensions.width;
  rp_begin.renderArea.extent.height = context_->swapchainDimensions.height;
  rp_begin.clearValueCount = clearColor.size();
  rp_begin.pClearValues = clearColor.data();

  vkCmdBeginRenderPass(cmd_buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

  renderCubemap(cmd_buffer, index);
  int64_t padding = dev::StaticHelpers::padUniformBufferOffset(context_, sizeof(UniformBlocks));
  for (auto& draw_call : resources_->draw_calls) {
    DrawCmd cmd;
    cmd.Execute(cmd_buffer, draw_call, index, padding);
  }

  vkCmdEndRenderPass(cmd_buffer);
  vkEndCommandBuffer(cmd_buffer);
  /*if (context_->perFrame[index].swapchainRelease == VK_NULL_HANDLE) {
    VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    vkCreateSemaphore(context_->logDevice_, &semaphoreInfo, nullptr,
      &context_->perFrame[index].swapchainRelease);
  }*/
  resources_->draw_calls.clear();

  VkPipelineStageFlags waitStage{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

  VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmd_buffer;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &context_->perFrame[index].swapchainAcquire;
  submitInfo.pWaitDstStageMask = &waitStage;
  //submitInfo.signalSemaphoreCount = 1;
  //submitInfo.pSignalSemaphores = &context_->perFrame[index].swapchainRelease;

  vkQueueSubmit(context_->graphicsQueue, 1, &submitInfo, context_->perFrame[index].submitFence);

}

void VulkanApp::renderCubemap(VkCommandBuffer cmd_buffer, uint32 index)
{
  InternalMaterial* internalMat = &resources_->cubemapPipeline;
  VkDeviceSize offsets[] = { 0 };
  VkBuffer vertexBuffers[] = { resources_->vertexBuffer.buffer_ };

  vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, internalMat->matPipeline);
  vkCmdBindVertexBuffers(cmd_buffer, 0, 1, vertexBuffers, offsets);
  vkCmdBindIndexBuffer(cmd_buffer, resources_->indicesBuffer.buffer_, 0, VK_INDEX_TYPE_UINT32);
  vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    resources_->layouts[internalMat->layout].pipeline, 0, 1,
    &internalMat->matDescriptorSet[index], 0, NULL);

  InternalVertexData vertex_data = resources_->vertex_data[(uint32)PrimitiveType::kPrimitiveType_Cube];
  uint32 first_vertex = vertex_data.offset;
  uint32 first_index = vertex_data.index_offset;
  vkCmdDrawIndexed(cmd_buffer, static_cast<uint32>(vertex_data.indices.size()), 1, first_index, first_vertex, 0);
}

/*********************************************************************************************/

int32 VulkanApp::presentImage(uint32 index)
{
  VkPresentInfoKHR present{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
  present.swapchainCount = 1;
  present.pSwapchains = &context_->swapChain;
  present.pImageIndices = &index;
  present.swapchainCount = 1;
  //present.pWaitSemaphores = &context_->perFrame[index].swapchainRelease;

  int32 result = vkQueuePresentKHR(context_->graphicsQueue, &present);

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


void VulkanApp::initCubemap()
{
  resources_->cubemapTexture.loadCubemapKtx(context_, "./../../data/textures/cubemaps/lyckseleunorm.ktx", VK_FORMAT_R8G8B8A8_SRGB);

  InternalMaterial* material = &resources_->cubemapPipeline;
  material->layout = kLayoutType_Texture_Cubemap;
  material->matPipeline = dev::StaticHelpers::createPipeline(context_, 
                                                             "./../../src/shaders/spir-v/skybox_vert.spv",
                                                             "./../../src/shaders/spir-v/skybox_frag.spv",
                                                             resources_->layouts[kLayoutType_Texture_Cubemap].pipeline, 
                                                             VK_CULL_MODE_FRONT_BIT, VK_FALSE);


  uint32 descriptor_size = static_cast<uint32>(context_->swapchainImageViews.size());
  std::vector<VkDescriptorPoolSize> poolSizes{ { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , descriptor_size },
                                               { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , descriptor_size } };

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = descriptor_size;

  if (vkCreateDescriptorPool(context_->logDevice_, &poolInfo, nullptr,
    &material->matDesciptorPool) != VK_SUCCESS) {

    throw std::runtime_error("Failed to create descriptor pool");
  }


  std::vector<VkDescriptorSetLayout> layouts(context_->swapchainImageViews.size(),
                                 resources_->layouts[material->layout].descriptor);

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = material->matDesciptorPool;
  allocInfo.descriptorSetCount = descriptor_size;
  allocInfo.pSetLayouts = layouts.data();
  material->matDescriptorSet.resize(descriptor_size);
  if (vkAllocateDescriptorSets(context_->logDevice_, &allocInfo,
    material->matDescriptorSet.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate descriptor sets");
  }

  VkDescriptorImageInfo img_info{};
  img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  vkdev::VkTexture* tex = &resources_->cubemapTexture;
  img_info.imageView = tex->view_;
  img_info.sampler = tex->sampler_;

  VkDescriptorBufferInfo bufferSceneInfo{};
  bufferSceneInfo.offset = 0;
  bufferSceneInfo.range = sizeof(SceneUniformBuffer);

  for (size_t i = 0; i < descriptor_size; i++) {
    bufferSceneInfo.buffer = resources_->staticUniform[i].buffer_;
    std::vector<VkWriteDescriptorSet> descriptor_write;

    descriptor_write.resize(2);
    descriptor_write[0] = dev::StaticHelpers::descriptorWriteInitializer(0,
                                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                             material->matDescriptorSet[i],
                                                         &bufferSceneInfo);

    descriptor_write[1] = dev::StaticHelpers::descriptorWriteInitializer(1,
                                 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                             material->matDescriptorSet[i],
                                                                &img_info);

    vkUpdateDescriptorSets(context_->logDevice_, descriptor_write.size(), descriptor_write.data(), 0, nullptr);

  }

}

/*********************************************************************************************/

VulkanApp::VulkanApp()
{
  context_ = new Context();
  debug_data_ = new DebugUtils();
  user_app_ = new UserMain();
  context_->physDevice_ = VK_NULL_HANDLE;
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
  ResourceManager::Get()->initPrimitiveGeometries();
  user_app_->init();
  resources_ = ResourceManager::Get()->getResources();
  createAppInstance();
  setupDebugMessenger();
  createSurface();
  setupPhysicalDevice();
  createLogicalDevice();
  createSwapChain();
  createRenderPass();
  createDescriptorSetLayout();
  createPipelineLayout();
  createInternalMaterials();
  createCommandPool();
  createDepthResource();
  createFramebuffer();
  storeTextures();
  createVertexBuffers();
  createIndexBuffers();
  createUniformBuffers();
  initCubemap();
  createDescriptorPool();
  createDescriptorSets();
}

/*********************************************************************************************/

void VulkanApp::loop()
{ 
  Scene::lastTime = std::chrono::high_resolution_clock::now();
  while (!glfwWindowShouldClose(context_->window_)) {
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>
                                  (currentTime - Scene::lastTime).count();
    glfwPollEvents();
    Scene::camera.cameraInput(deltaTime);
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

  for (auto& material : resources_->internalMaterials) {
    dev::StaticHelpers::destroyMaterial(context_, &material);
  }

  dev::StaticHelpers::destroyMaterial(context_, &resources_->cubemapPipeline);
  resources_->cubemapTexture.destroyTexture();

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
