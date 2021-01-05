#include "vulkan_app.h"
#include "internal.h"
#include "static_helpers.h"
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


//////////////////////////////////////////////////////////////////////////////////////////////

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData) {

  printf("validation layer: %s\n", pCallbackData->pMessage);

  return VK_FALSE;
}

static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerInfo) {

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
  appInfo.pApplicationName = "VulkanTest";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "My Vulkan Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  if (enableValidationLayers) {
    createInfo.enabledLayerCount = validationLayers.size();
    createInfo.ppEnabledLayerNames = validationLayers.data();
    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }
  
  /**Get required extensions**/
  uint32 glfwExtensionCount = 0;
  const char** glfwExtension;
  glfwExtension = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(glfwExtension, glfwExtension + glfwExtensionCount);

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  createInfo.enabledExtensionCount = static_cast<uint32>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();


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

  QueueFamilyIndices indices = StaticHelpers::findQueueFamilies(device, context.surface);
  bool extensionSupported = checkDeviceExtensionSupport(device);
  SwapChainSupportDetails swapChainSupport = StaticHelpers::querySwapChain(device, context.surface);
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

  if (candidates.rbegin()->first > 0) {
    context_->physDevice_ = candidates.rbegin()->second;
    return;
  }

  abort();
}

/**********************************LOGICAL DEVICE*********************************************/

void VulkanApp::createLogicalDevice()
{
  QueueFamilyIndices indices = StaticHelpers::findQueueFamilies(context_->physDevice_, context_->surface);

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
  /*assert(*/glfwCreateWindowSurface(context_->instance_, context_->window_, nullptr, &context_->surface)/* == VK_SUCCESS)*/;
}

/************************************************************************************************/

void VulkanApp::createSwapChain()
{
  SwapChainSupportDetails swapChainSupport = StaticHelpers::querySwapChain(context_->physDevice_, context_->surface);

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

  QueueFamilyIndices indices = StaticHelpers::findQueueFamilies(context_->physDevice_, context_->surface);
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

  //assert(vkCreateSwapchainKHR(context_->logDevice_, &swapChainInfo, nullptr, &context_->swapChain) == VK_SUCCESS);
  vkCreateSwapchainKHR(context_->logDevice_, &swapChainInfo, nullptr, &context_->swapChain);

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

  for (size_t i = 0; i < swapImageCount; i++) {
    VkImageViewCreateInfo imageViewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = swap_chain_images[i];
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; //1D, 2D, 3D, CUBEMAP
    imageViewInfo.format = context_->swapchainDimensions.format;

    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;

    VkImageView image_view;
    //assert(vkCreateImageView(context_->logDevice_, &imageViewInfo, nullptr, &image_view) == VK_SUCCESS);
    vkCreateImageView(context_->logDevice_, &imageViewInfo, nullptr, &image_view);
    context_->swapchainImageViews.push_back(image_view);
  }

}

/*********************************************************************************************/

void VulkanApp::createPipelineLayout()
{
  Resources* res = ResourceManager::Get()->getResources();

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &res->descSetLayout;
  //pipelineLayoutInfo.pushConstantRangeCount = 0;
  //pipelineLayoutInfo.pPushConstantRanges = nullptr;

  vkCreatePipelineLayout(context_->logDevice_, &pipelineLayoutInfo, nullptr, &res->pipelineLayout);
}

/*********************************************************************************************/

void VulkanApp::createInternalMaterials()
{
  Resources* res = ResourceManager::Get()->getResources();
  InternalMaterial* material = &res->internalMaterials[kMaterialType_UnlitColor];
  material->matPipeline = StaticHelpers::createPipeline(context_, "./../../src/shaders/unlit_color_vert.spv", 
                                                        "./../../src/shaders/unlit_color_frag.spv",
                                                        res->pipelineLayout);

  material = &res->internalMaterials[kMaterialType_TextureSampler];
  material->matPipeline = StaticHelpers::createPipeline(context_, "./../../src/shaders/texture_sampling_vert.spv",
                                                        "./../../src/shaders/texture_sampling_frag.spv",
                                                        res->pipelineLayout);
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

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;

  /*SUBPASS*/
  VkSubpassDependency subpassDependency{};
  subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependency.dstSubpass = 0;
  subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.srcAccessMask = 0;
  subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &subpassDependency;

  //assert(vkCreateRenderPass(context_->logDevice_, &renderPassInfo, nullptr, &context_->renderPass) == VK_SUCCESS);
  vkCreateRenderPass(context_->logDevice_, &renderPassInfo, nullptr, &context_->renderPass);

}

/*********************************************************************************************/

void VulkanApp::createFramebuffer()
{
  int32 imageCount = context_->swapchainImageViews.size();
  context_->swapchainFramebuffers.resize(imageCount);

  for (size_t i = 0; i < imageCount; i++) {
    VkImageView attachment{ context_->swapchainImageViews[i] };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = context_->renderPass;
    framebufferInfo.attachmentCount = 1;

    /// !!!!!!!!!!!!!!!!!!!!!! ///
    framebufferInfo.pAttachments = &attachment;
    framebufferInfo.width = context_->swapchainDimensions.width;
    framebufferInfo.height = context_->swapchainDimensions.height;
    framebufferInfo.layers = 1;

    //assert(vkCreateFramebuffer(context_->logDevice_, &framebufferInfo, nullptr, &context_->swapchainFramebuffers[i]) == VK_SUCCESS);
    vkCreateFramebuffer(context_->logDevice_, &framebufferInfo, nullptr, &context_->swapchainFramebuffers[i]);
  }
}

/*********************************************************************************************/

void VulkanApp::createCommandPool()
{
  QueueFamilyIndices queueIndices = StaticHelpers::findQueueFamilies(context_->physDevice_, context_->surface);

  VkCommandPoolCreateInfo commandPoolInfo{};
  commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolInfo.queueFamilyIndex = queueIndices.graphicsFamily;

  commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  //assert(vkCreateCommandPool(context_->logDevice_, &commandPoolInfo, nullptr, &context_->transferCommandPool) == VK_SUCCESS);
  vkCreateCommandPool(context_->logDevice_, &commandPoolInfo, nullptr, &context_->transferCommandPool);

}

/*********************************************************************************************/

void VulkanApp::storeTextures()
{
  Resources* res = ResourceManager::Get()->getResources();
  uint32 textures_number = Scene::textureCount;
  res->internalTextures.resize(textures_number);

  for (size_t i = 0; i < textures_number; i++) {
    InternalTexture new_texture = StaticHelpers::createTextureImage(context_, Scene::userTextures[i]);
    res->internalTextures[i] = new_texture;
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
  StaticHelpers::createInternalBuffer(*context_, total_vertex_bytes, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mainResources->bufferObject, mainResources->vertexBufferMemory);

  for (size_t i = 0; i < geometry_number; i++) {
    InternalVertexData* vertexData = &mainResources->vertex_data[i];

    VkDeviceSize size = sizes[i];
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    StaticHelpers::createInternalBuffer(*context_, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(context_->logDevice_, stagingBufferMemory, 0, size, 0, &data);
    memcpy(data, vertexData->vertex.data(), size);
    vkUnmapMemory(context_->logDevice_, stagingBufferMemory);

    StaticHelpers::copyBuffer(*context_, stagingBuffer, mainResources->bufferObject, size, offset_bytes[i]);
    vkDestroyBuffer(context_->logDevice_, stagingBuffer, nullptr);
    vkFreeMemory(context_->logDevice_, stagingBufferMemory, nullptr);
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
  StaticHelpers::createInternalBuffer(*context_, total_index_bytes, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mainResources->bufferIndices, mainResources->indexBufferMemory);

  for (size_t i = 0; i < geometry_number; i++) {
    InternalVertexData* vertexData = &mainResources->vertex_data[i];

    VkDeviceSize size = sizes[i];
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    StaticHelpers::createInternalBuffer(*context_, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(context_->logDevice_, stagingBufferMemory, 0, size, 0, &data);
    memcpy(data, vertexData->indices.data(), size);
    vkUnmapMemory(context_->logDevice_, stagingBufferMemory);

    StaticHelpers::copyBuffer(*context_, stagingBuffer, mainResources->bufferIndices, size, offset_bytes[i]);
    vkDestroyBuffer(context_->logDevice_, stagingBuffer, nullptr);
    vkFreeMemory(context_->logDevice_, stagingBufferMemory, nullptr);
  }
}

/*********************************************************************************************/

void VulkanApp::createDescriptorSetLayout()
{
  Resources* res = ResourceManager::Get()->getResources();

  std::vector<VkDescriptorSetLayoutBinding> textureLayoutBinding(3);
  textureLayoutBinding[0].binding = 0;
  textureLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  textureLayoutBinding[0].descriptorCount = 1;
  textureLayoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  textureLayoutBinding[0].pImmutableSamplers = nullptr;

  textureLayoutBinding[1].binding = 1;
  textureLayoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  textureLayoutBinding[1].descriptorCount = 1;
  textureLayoutBinding[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  textureLayoutBinding[1].pImmutableSamplers = nullptr;

  textureLayoutBinding[2].binding = 2;
  textureLayoutBinding[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  textureLayoutBinding[2].descriptorCount = kTexturePerShader;
  textureLayoutBinding[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  textureLayoutBinding[2].pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32>(textureLayoutBinding.size());
  layoutInfo.pBindings = textureLayoutBinding.data();

  if (vkCreateDescriptorSetLayout(context_->logDevice_, &layoutInfo, nullptr, 
                                  &res->descSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor set layout");
  }
  
}

/*********************************************************************************************/

void VulkanApp::createDescriptorPool()
{
  Resources* res = ResourceManager::Get()->getResources();

  for (size_t i = 0; i < kMaterialType_MAX; i++) {
    std::vector<VkDescriptorPoolSize> texPoolSizes{
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , static_cast<uint32>(context_->swapchainImageViews.size())},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC , static_cast<uint32>(context_->swapchainImageViews.size())}
    };

    if (i >= kMaterialType_TextureSampler) {
      VkDescriptorPoolSize pool{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , static_cast<uint32>(context_->swapchainImageViews.size()) };
      texPoolSizes.push_back(pool);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32>(texPoolSizes.size());
    poolInfo.pPoolSizes = texPoolSizes.data();

    poolInfo.maxSets = static_cast<uint32>(context_->swapchainImageViews.size());

    if (vkCreateDescriptorPool(context_->logDevice_, &poolInfo, nullptr,
      &res->internalMaterials[i].matDesciptorPool) != VK_SUCCESS) {

      throw std::runtime_error("Failed to create descriptor pool");
    }
  }
}

/*********************************************************************************************/

void VulkanApp::createDescriptorSets()
{
  Resources* resources = ResourceManager::Get()->getResources();
  std::vector<VkDescriptorSetLayout> layouts(context_->swapchainImageViews.size(), resources->descSetLayout);

  for (size_t j = 0; j < kMaterialType_MAX; j++) {
    InternalMaterial* mat = &resources->internalMaterials[j];
    if (!mat->uniformBuffers.empty()) {
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

      VkWriteDescriptorSet image_descriptor;
      std::vector<VkDescriptorImageInfo> image_info;
      uint32 texture_number = mat->texturesReferenced.size();
      image_info.resize(texture_number);
      for (size_t i = 0; i < texture_number; i++) {
        VkDescriptorImageInfo img_info{};
        img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        InternalTexture* tex = &resources->internalTextures[mat->texturesReferenced[i]];
        img_info.imageView = tex->textureImageView;
        img_info.sampler = tex->textureSampler;
        image_info[i] = img_info;
      }

      uint32 bind_number = 3;
      for (size_t i = 0; i < context_->swapchainImageViews.size(); i++) {
        VkDescriptorBufferInfo bufferSceneInfo{};
        bufferSceneInfo.buffer = resources->sceneUniformBuffers[i];
        bufferSceneInfo.offset = 0;
        bufferSceneInfo.range = sizeof(SceneUniformBuffer);

        std::vector<VkWriteDescriptorSet> descriptorWrite(bind_number);
        descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[0].dstSet = mat->matDescriptorSet[i];
        descriptorWrite[0].dstBinding = 0;
        descriptorWrite[0].dstArrayElement = 0;
        descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite[0].descriptorCount = 1;
        descriptorWrite[0].pBufferInfo = &bufferSceneInfo;
        descriptorWrite[0].pImageInfo = nullptr;
        descriptorWrite[0].pTexelBufferView = nullptr;


        VkDescriptorBufferInfo bufferObjectInfo{};
        bufferObjectInfo.buffer = mat->uniformBuffers[i];
        bufferObjectInfo.offset = 0;
        bufferObjectInfo.range = sizeof(UniformBlocks);

        descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[1].dstSet = mat->matDescriptorSet[i];
        descriptorWrite[1].dstBinding = 1;
        descriptorWrite[1].dstArrayElement = 0;
        descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrite[1].descriptorCount = 1;
        descriptorWrite[1].pBufferInfo = &bufferObjectInfo;
        descriptorWrite[1].pImageInfo = nullptr;
        descriptorWrite[1].pTexelBufferView = nullptr;


        descriptorWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[2].dstBinding = 2;
        descriptorWrite[2].dstSet = mat->matDescriptorSet[i];
        descriptorWrite[2].dstArrayElement = 0;
        descriptorWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite[2].descriptorCount = image_info.size();
        descriptorWrite[2].pImageInfo = image_info.data();

        uint32 descriptor_count = bind_number;
        if (j < kMaterialType_TextureSampler) descriptor_count = 2;

        vkUpdateDescriptorSets(context_->logDevice_, descriptor_count, descriptorWrite.data(), 0, nullptr);
      }
    }
  }
}

/*********************************************************************************************/

void VulkanApp::createUniformBuffers()
{
  uint64_t dynamicAlignment = StaticHelpers::padUniformBufferOffset(context_, sizeof(UniformBlocks));

  Resources* resources = ResourceManager::Get()->getResources();
  uint32 swapChainImageCount = context_->swapchainImageViews.size();

  for (size_t j = 0; j < kMaterialType_MAX; j++) {
    InternalMaterial* mat = &resources->internalMaterials[j];
    mat->dynamicUniformMapped.resize(swapChainImageCount);
    if (mat->entitiesReferenced) {
    VkDeviceSize dynamicBufferSize = dynamicAlignment * mat->entitiesReferenced;
    mat->uniformBuffers.resize(swapChainImageCount);
    mat->uniformBufferMemory.resize(swapChainImageCount);
      mat->dynamicUniformData = (UniformBlocks*)_aligned_malloc(dynamicBufferSize, dynamicAlignment);
      for (size_t i = 0; i < swapChainImageCount; i++) {
        StaticHelpers::createInternalBuffer(*context_, dynamicBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, mat->uniformBuffers[i], mat->uniformBufferMemory[i]);

        vkMapMemory(context_->logDevice_, mat->uniformBufferMemory[i], 0, dynamicBufferSize, 0, &mat->dynamicUniformMapped[i]);
      }
    }
  }

  resources->sceneUniformBuffers.resize(swapChainImageCount);
  resources->staticUniformMapped.resize(swapChainImageCount);
  resources->sceneUniformBufferMemory.resize(swapChainImageCount);
  for (size_t i = 0; i < swapChainImageCount; i++) {
    StaticHelpers::createInternalBuffer(*context_, sizeof(SceneUniformBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                        resources->sceneUniformBuffers[i], resources->sceneUniformBufferMemory[i]);

    vkMapMemory(context_->logDevice_, resources->sceneUniformBufferMemory[i], 0, sizeof(SceneUniformBuffer), 0, &resources->staticUniformMapped[i]);
  }
}

/*********************************************************************************************/

void VulkanApp::updateUniformBuffers(uint32 index)
{
  Resources* resources = ResourceManager::Get()->getResources();

  SceneUniformBuffer sceneBuffer{ Scene::camera.getView(), Scene::camera.getProjection() };
  memcpy(resources->staticUniformMapped[index], &sceneBuffer, sizeof(SceneUniformBuffer));
  VkMappedMemoryRange memoryRange{ VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE };
  memoryRange.memory = resources->sceneUniformBufferMemory[index];
  memoryRange.size = sizeof(SceneUniformBuffer);
  vkFlushMappedMemoryRanges(context_->logDevice_, 1, &memoryRange);

  for (auto& entity : Scene::sceneEntities) {
    UniformBlocks u_blocks{};
    Material* mat = entity.getMaterial();
    if (mat) {
      u_blocks = mat->getMaterialSettings();
      u_blocks.unlitBlock.model = glm::mat4(1.0f);
      Transform* tr = entity.getComponent<Transform>(kComponentType_Transform);
      if (tr) u_blocks.unlitBlock.model = tr->getModel();
      int32 offset = entity.getMaterialOffset();
      UniformBlocks* buffer = (UniformBlocks*)((uint64_t)resources->internalMaterials[mat->getMaterialType()].dynamicUniformData + 
                                                (offset * StaticHelpers::padUniformBufferOffset(context_, sizeof(UniformBlocks))));
      *buffer = u_blocks;
    }
  }

  for (auto& internal_material : resources->internalMaterials) {
    //if (internal_material.entitiesReferenced) { ///////////////////////////////////////// TODO
      uint64_t sceneUboSize = internal_material.entitiesReferenced * StaticHelpers::padUniformBufferOffset(context_, sizeof(UniformBlocks));
      memcpy(internal_material.dynamicUniformMapped[index], internal_material.dynamicUniformData, sceneUboSize);
      //memoryRange.memory = internal_material.uniformBufferMemory[index];
      //memoryRange.size = sceneUboSize;
      //vkFlushMappedMemoryRanges(context_->logDevice_, 1, &memoryRange);
    //}
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
    QueueFamilyIndices graphicIndices = StaticHelpers::findQueueFamilies(context_->physDevice_, context_->surface);
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
  
  VkClearValue clearColor;
  clearColor.color = { 0.1f, 0.1f, 0.2f, 1.0f };

  VkRenderPassBeginInfo rp_begin{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
  rp_begin.renderPass = context_->renderPass;
  rp_begin.framebuffer = framebuffer;
  rp_begin.renderArea.extent.width = context_->swapchainDimensions.width;
  rp_begin.renderArea.extent.height = context_->swapchainDimensions.height;
  rp_begin.clearValueCount = 1;
  rp_begin.pClearValues = &clearColor;

  vkCmdBeginRenderPass(cmd_buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

  Resources* intResources = ResourceManager::Get()->getResources();

  VkBuffer vertexBuffers[] = { intResources->bufferObject };
  VkDeviceSize offsets[] = { 0 };
  for (auto& entity : Scene::sceneEntities) {
    Material* mat = entity.getMaterial();
    if (mat) {
      int32 matType = mat->getMaterialType();
      uint32 uniform_offset = entity.getMaterialOffset() * StaticHelpers::padUniformBufferOffset(context_, sizeof(UniformBlocks));
      InternalMaterial* internalMat = &intResources->internalMaterials[matType];

      vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, internalMat->matPipeline);
      vkCmdBindVertexBuffers(cmd_buffer, 0, 1, vertexBuffers, offsets);
      vkCmdBindIndexBuffer(cmd_buffer, intResources->bufferIndices, 0, VK_INDEX_TYPE_UINT32);
      vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              intResources->pipelineLayout, 0, 1,
                              &internalMat->matDescriptorSet[index], 1, &uniform_offset);

      Geometry* geo = entity.getComponent<Geometry>(kComponentType_Geometry);
      if (geo && geo->getId() >= 0) {
        InternalVertexData vertex_data = intResources->vertex_data[geo->getId()];
        uint32 first_vertex = vertex_data.offset;
        uint32 first_index = vertex_data.index_offset;
        vkCmdDrawIndexed(cmd_buffer, static_cast<uint32>(vertex_data.indices.size()), 1, first_index, first_vertex, 0);
      }
    }
  }

  vkCmdEndRenderPass(cmd_buffer);
  vkEndCommandBuffer(cmd_buffer);
  if (context_->perFrame[index].swapchainRelease == VK_NULL_HANDLE) {
    VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    /*assert(vkCreateSemaphore(context_->logDevice_, &semaphoreInfo, nullptr, 
                             &context_->perFrame[index].swapchainRelease) == VK_SUCCESS);*/
    vkCreateSemaphore(context_->logDevice_, &semaphoreInfo, nullptr,
      &context_->perFrame[index].swapchainRelease);
  }

  VkPipelineStageFlags waitStage{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

  VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmd_buffer;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &context_->perFrame[index].swapchainAcquire;
  submitInfo.pWaitDstStageMask = &waitStage;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &context_->perFrame[index].swapchainRelease;

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
  present.pWaitSemaphores = &context_->perFrame[index].swapchainRelease;

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
  context_->window_ = glfwCreateWindow(k_wWidth, k_wHeight, "VulkanTest", nullptr, nullptr);
  glfwSetKeyCallback(context_->window_, InputManager::keyCallback);
  glfwSetInputMode(context_->window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(context_->window_, InputManager::mouseCallback);
  ResourceManager::Get()->initPrimitiveGeometries();
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
  createInternalMaterials();
  createFramebuffer();
  createCommandPool();
  storeTextures();
  createVertexBuffers();
  createIndexBuffers();
  createUniformBuffers();
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
  context_->perFrame.clear();

  ResourceManager* rm = ResourceManager::Get();
  Resources* intResources = rm->getResources();
  for (auto& material : intResources->internalMaterials) {
    StaticHelpers::destroyMaterial(context_, &material);
  }

  vkDestroyRenderPass(context_->logDevice_, context_->renderPass, nullptr);

  //Swap chain
  for (auto image_view : context_->swapchainImageViews) {
    vkDestroyImageView(context_->logDevice_, image_view, nullptr);
  }
  vkDestroySwapchainKHR(context_->logDevice_, context_->swapChain, nullptr);

  //Textures
  for (auto& texture : intResources->internalTextures) {
    vkDestroySampler(context_->logDevice_, texture.textureSampler, nullptr);
    vkDestroyImageView(context_->logDevice_, texture.textureImageView, nullptr);
    vkDestroyImage(context_->logDevice_, texture.textureImage, nullptr);
    vkFreeMemory(context_->logDevice_, texture.textureImageMemory, nullptr);
  }

  //Vertex Buffers
  vkDestroyBuffer(context_->logDevice_, intResources->bufferIndices, nullptr);
  vkFreeMemory(context_->logDevice_, intResources->indexBufferMemory, nullptr);
  vkDestroyBuffer(context_->logDevice_, intResources->bufferObject, nullptr);
  vkFreeMemory(context_->logDevice_, intResources->vertexBufferMemory, nullptr);
  

  vkDestroySurfaceKHR(context_->instance_, context_->surface, nullptr);
  vkDestroyDevice(context_->logDevice_, nullptr);
  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(context_->instance_, debug_data_->debugMessenger_, nullptr);
  }
  vkDestroyInstance(context_->instance_, nullptr);
  delete(rm);

  glfwDestroyWindow(context_->window_);
  glfwTerminate();
}

/*********************************************************************************************/
