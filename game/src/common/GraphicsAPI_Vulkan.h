// Copyright 2023, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

// OpenXR Tutorial for Khronos Group

#pragma once

#include "GraphicsAPI.h"
#include "deletion_queue.h"

#if defined(XR_USE_GRAPHICS_API_VULKAN)
#define VULKAN_CHECK(x, y)                                                                         \
    {                                                                                              \
        VkResult result = (x);                                                                     \
        if (result != VK_SUCCESS) {                                                                \
            std::cout << "ERROR: VULKAN: " << std::hex << "0x" << result << std::dec << std::endl; \
            std::cout << "ERROR: VULKAN: "  << string_VkResult(result) << std::endl; \
            std::cout << "ERROR: VULKAN: " << y << std::endl;                                      \
        }                                                                                          \
    }


#define VULKAN_CHECK_NOMSG(x)                                                                         \
    {                                                                                              \
        VkResult result = (x);                                                                     \
        if (result != VK_SUCCESS) {                                                                \
            std::cout << "ERROR: VULKAN: " << std::hex << "0x" << result << std::dec << std::endl; \
            std::cout << "ERROR: VULKAN: "  << string_VkResult(result) << std::endl; \
        }                                                                                          \
    }


class GraphicsAPI_Vulkan : public GraphicsAPI {
public:
    struct UploadContext {
        VkFence uploadFence;
        VkCommandPool pool;
        VkCommandBuffer buffer;
    };

    // sample resources
    GraphicsAPI::Image tex_placeholder;

    UploadContext m_uploadContext;

    void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
public:
    GraphicsAPI_Vulkan(XrInstance m_xrInstance, XrSystemId systemId);
    ~GraphicsAPI_Vulkan();

    void* CreateDesktopSwapchain(const GraphicsAPI::SwapchainCreateInfo& swapchainCI) ;
    void DestroyDesktopSwapchain(void*& swapchain) ;
    void* GetDesktopSwapchainImage(void* swapchain, uint32_t index) ;
    void AcquireDesktopSwapchanImage(void* swapchain, uint32_t& index) ;
    void PresentDesktopSwapchainImage(void* swapchain, uint32_t index) ;

    // XR_DOCS_TAG_BEGIN_GetDepthFormat_Vulkan
    int64_t GetDepthFormat()  { return (int64_t)VK_FORMAT_D32_SFLOAT; }
    // XR_DOCS_TAG_END_GetDepthFormat_Vulkan

    void* GetGraphicsBinding() ;
    XrSwapchainImageBaseHeader* AllocateSwapchainImageData(XrSwapchain swapchain, GraphicsAPI::SwapchainType type, uint32_t count) ;
    void FreeSwapchainImageData(XrSwapchain swapchain)  {
        swapchainImagesMap[swapchain].second.clear();
        swapchainImagesMap.erase(swapchain);
    }
    XrSwapchainImageBaseHeader* GetSwapchainImageData(XrSwapchain swapchain, uint32_t index)  { return (XrSwapchainImageBaseHeader*)&swapchainImagesMap[swapchain].second[index]; }
    // XR_DOCS_TAG_BEGIN_GetSwapchainImage_Vulkan
    void* GetSwapchainImage(XrSwapchain swapchain, uint32_t index)  {
        VkImage image = swapchainImagesMap[swapchain].second[index].image;
        VkImageLayout layout = swapchainImagesMap[swapchain].first == GraphicsAPI::SwapchainType::COLOR ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        imageStates[image] = layout;
        return (void *)image;
    }
    // XR_DOCS_TAG_END_GetSwapchainImage_Vulkan

    VkImage CreateImage(const GraphicsAPI::ImageCreateInfo& imageCI) ;
    void DestroyImage(VkImage image) ;

    VkImageView CreateImageView(const GraphicsAPI::ImageViewCreateInfo& imageViewCI) ;
    void DestroyImageView(VkImageView imageView) ;

    VkSampler* CreateSampler(const GraphicsAPI::SamplerCreateInfo& samplerCI) ;
    void DestroySampler(VkSampler sampler) ;

    VkBuffer CreateBuffer(const GraphicsAPI::BufferCreateInfo& bufferCI) ;
    void DestroyBuffer(VkBuffer buffer) ;

    Shader CreateShader(const GraphicsAPI::ShaderCreateInfo& shaderCI) ;
    void DestroyShader(Shader shader) ;

    GraphicsAPI::Pipeline  CreatePipeline(const GraphicsAPI::PipelineCreateInfo& pipelineCI) ;
    void DestroyPipeline(GraphicsAPI::Pipeline  pipeline) ;

    void BeginRendering() ;
    void EndRendering() ;

    void SetBufferData(VkBuffer buffer, size_t offset, size_t size, void* data) ;
    void PushConstant(void* data, size_t size, VkShaderStageFlagBits stage);

    void ClearColor(VkImageView& imageView, float r, float g, float b, float a) ;
    void ClearDepth(void* imageView, float d) ;

    void SetRenderAttachments(VkImageView colorViews, size_t colorViewCount, VkImageView depthStencilView, uint32_t width, uint32_t height, GraphicsAPI::Pipeline pipeline) ;
    void SetViewports(GraphicsAPI::Viewport* viewports, size_t count) ;
    void SetScissors(GraphicsAPI::Rect2D* scissors, size_t count) ;

    void SetPipeline(GraphicsAPI::Pipeline  pipeline) ;
    void SetDescriptor(const GraphicsAPI::DescriptorInfo& descriptorInfo) ;
    void UpdateDescriptors() ;
    void SetVertexBuffers(VkBuffer* vertexBuffers, size_t count) ;
    void SetIndexBuffer(VkBuffer indexBuffer) ;
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) ;
    void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) ;

    void SetDebugName(std::string name, VkBuffer object);
	void SetDebugName(std::string name, VkImage object);
	void SetDebugName(std::string name, VkImageView object);
	void SetDebugName(std::string name, VkRenderPass object);
	void SetDebugName(std::string name, VkCommandBuffer object);




    VkDevice GetDevice() { return device; }
    VmaAllocator GetAllocator() { return m_allocator; }

    DeletionQueue MainDeletionQueue;
private:
    void LoadPFN_XrFunctions(XrInstance m_xrInstance);
    std::vector<std::string> GetInstanceExtensionsForOpenXR(XrInstance m_xrInstance, XrSystemId systemId);
    std::vector<std::string> GetDeviceExtensionsForOpenXR(XrInstance m_xrInstance, XrSystemId systemId);

    const std::vector<int64_t> GetSupportedColorSwapchainFormats() ;
    const std::vector<int64_t> GetSupportedDepthSwapchainFormats() ;

private:
    VkInstance instance{};
    VkPhysicalDevice physicalDevice{};
    VkDevice device{};
    uint32_t queueFamilyIndex = 0xFFFFFFFF;
    uint32_t queueIndex = 0xFFFFFFFF;
    VkQueue queue{};
    VkFence fence{};

    VkCommandPool cmdPool{};
    VkCommandBuffer cmdBuffer{};
    VkDescriptorPool descriptorPool;

    std::vector<const char*> activeInstanceLayers{};
    std::vector<const char*> activeInstanceExtensions{};
    std::vector<const char*> activeDeviceLayer{};
    std::vector<const char*> activeDeviceExtensions{};

    PFN_xrGetVulkanGraphicsRequirementsKHR xrGetVulkanGraphicsRequirementsKHR = nullptr;
    PFN_xrGetVulkanInstanceExtensionsKHR xrGetVulkanInstanceExtensionsKHR = nullptr;
    PFN_xrGetVulkanDeviceExtensionsKHR xrGetVulkanDeviceExtensionsKHR = nullptr;
    PFN_xrGetVulkanGraphicsDeviceKHR xrGetVulkanGraphicsDeviceKHR = nullptr;

	PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = VK_NULL_HANDLE;
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = VK_NULL_HANDLE;

	VkDebugUtilsMessengerEXT debugMessenger;

    XrGraphicsBindingVulkanKHR graphicsBinding{};

    std::unordered_map<XrSwapchain, std::pair<GraphicsAPI::SwapchainType, std::vector<XrSwapchainImageVulkanKHR>>> swapchainImagesMap{};

    VkImage currentDesktopSwapchainImage = VK_NULL_HANDLE;

    std::unordered_map<VkSwapchainKHR, VkSurfaceKHR> surfaces;
    VkSemaphore acquireSemaphore{};
    VkSemaphore submitSemaphore{};

    std::unordered_map<VkImage, VkImageLayout> imageStates;
    std::unordered_map<VkImage, std::pair<VmaAllocation, GraphicsAPI::ImageCreateInfo>> imageResources;
    std::unordered_map<VkImageView, GraphicsAPI::ImageViewCreateInfo> imageViewResources;
    
    std::unordered_map<VkBuffer, std::pair<VmaAllocation, GraphicsAPI::BufferCreateInfo>> bufferResources;

    std::unordered_map<VkPipeline, std::tuple<VkPipelineLayout, std::vector<VkDescriptorSetLayout>, VkRenderPass, GraphicsAPI::PipelineCreateInfo>> pipelineResources;

    std::unordered_map<VkCommandBuffer, std::vector<VkFramebuffer>> cmdBufferFramebuffers;
    bool inRenderPass = false;

    VkPipeline setPipeline = VK_NULL_HANDLE;
    std::unordered_map<VkCommandBuffer, std::vector<VkDescriptorSet>> cmdBufferDescriptorSets;
    std::vector<std::tuple<uint32_t, VkWriteDescriptorSet, VkDescriptorBufferInfo, VkDescriptorImageInfo>> writeDescSets;

    VmaAllocator m_allocator;
public:
    VkDescriptorPool GetDescriptorPool();
};
#endif