#pragma once

import vulkan_hpp;

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vertex.hh"
#include "vulkanUtils.hh"

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

constexpr uint32_t MAX_FRAME_IN_FLIGHT = 2;

struct MVPUniformBuffer {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class HelloTriangleApplicationCpp {
    GLFWwindow *window_;

    vk::raii::Context context_;

    vk::raii::Instance instance_ = nullptr;

    vk::raii::DebugUtilsMessengerEXT debug_messenger_ = nullptr;

    vk::raii::SurfaceKHR surface_ = nullptr;

    vk::raii::PhysicalDevice physical_device_ = nullptr;
    RequiredQueueFamilyIndices queue_family_indices_;

    vk::raii::Device device_ = nullptr;

    vk::raii::Queue graphic_queue_ = nullptr;
    vk::raii::Queue present_queue_ = nullptr;

    vk::SampleCountFlagBits msaa_samples_ = vk::SampleCountFlagBits::e1;

    uint32_t swapchain_min_image_count = 0;
    vk::raii::SwapchainKHR swapchain_ = nullptr;
    std::vector<vk::Image> swapchain_images_;
    vk::Format swapchain_image_format_ = vk::Format::eUndefined;
    vk::Extent2D swapchain_extent_;
    std::vector<vk::raii::ImageView> swapchain_image_views_;
    bool framebufferResized = false;

    vk::raii::Image color_buffer_ = nullptr;
    vk::raii::DeviceMemory color_buffer_memory_ = nullptr;
    vk::raii::ImageView color_buffer_image_view_ = nullptr;

    vk::raii::Image depth_buffer_ = nullptr;
    vk::raii::DeviceMemory depth_buffer_memory_ = nullptr;
    vk::raii::ImageView depth_buffer_image_view_ = nullptr;

    vk::raii::DescriptorSetLayout descriptor_set_layout_ = nullptr;
    vk::raii::PipelineLayout pipeline_layout_ = nullptr;
    vk::raii::Pipeline graphic_pipeline_ = nullptr;

    vk::raii::CommandPool command_pool_ = nullptr;
    vk::raii::CommandPool transient_command_pool_ = nullptr;
    vk::raii::CommandBuffers command_buffers_ = nullptr;

    std::vector<vk::raii::Semaphore> present_complete_semaphores_;
    std::vector<vk::raii::Semaphore> render_finished_semaphores_;
    std::vector<vk::raii::Fence> in_flight_fences_;

    uint32_t image_index_ = 0;

    std::vector<Vertex> vertices_;
    vk::raii::Buffer vertex_buffer_ = nullptr;
    vk::raii::DeviceMemory vertex_buffer_memory_ = nullptr;

    std::vector<uint32_t> indices_;
    vk::raii::Buffer index_buffer_ = nullptr;
    vk::raii::DeviceMemory index_buffer_memory_ = nullptr;

    std::vector<vk::raii::Buffer> mvp_uniform_buffers_;
    std::vector<vk::raii::DeviceMemory> mvp_uniform_buffers_memory_;
    std::vector<void *> mvp_uniform_buffers_mapped_;

    vk::raii::DescriptorPool descriptor_pool_ = nullptr;
    std::vector<vk::raii::DescriptorSet> descriptor_sets_;

    uint32_t texture_image_mip_levels_ = 0;
    vk::raii::Image texture_image_ = nullptr;
    vk::raii::DeviceMemory texture_image_memory_ = nullptr;
    vk::raii::ImageView texture_image_view_ = nullptr;
    vk::raii::Sampler texture_image_sampler_ = nullptr;

public:
    HelloTriangleApplicationCpp();

    ~HelloTriangleApplicationCpp();

    void run();

private:
    void initWindow();

    void initVulkan();

    void cleanup();

    void createInstance();

    void createSurface();

    void pickPhysicalDevice();
    void createLogicalDevice();

    void createSwapChain();
    void createSwapchainImageView();

    void createColorBufferResources();
    void createDepthBufferResources();

    void createDescriptorSetLayout();

    void createGraphicPipeline();

    void createCommandPool();

    void createTextureImage();
    void createTextureImageView();
    void createTextureImageSampler();

    void loadModel();
    void createVertexBuffer();
    void createIndexBuffer();

    void createUniformBuffers();

    void createDescriptorPool();
    void createDescriptorSets();

    void createCommandBuffers();
    void recordCommandBuffers() const;
    void recordCommandBuffer(uint32_t image_index) const;
    void recordImguiCommandBuffer(uint32_t image_index) const;

    void initImgui() const;

    void createSyncObject();

    //-------------------------

    void cleanupSwapChain();

    void recreateSwapChain();

    //-------------------------

    void drawFrame();

    void Update() const;

    //-------------------------

    void UpdateMVPUniformBuffer() const;
    void UpdateImGui() const;
    void UpdateCommandBuffer() const;

    //-------------------------

    void setupDebugMessenger();

    //-------------------------

    [[nodiscard]] vk::Extent2D chooseExtent2D(const vk::SurfaceCapabilitiesKHR& surface_capabilities) const;

    [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::filesystem::path& shader_path) const;

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

    void createBuffer(vk::DeviceSize buffer_size,
                      vk::BufferUsageFlags buffer_usage,
                      vk::MemoryPropertyFlags memory_properties,
                      vk::raii::Buffer &buffer,
                      vk::raii::DeviceMemory &buffer_memory) const;

    void copyBuffer(const vk::raii::Buffer &src,
                    const vk::raii::Buffer &dst,
                    vk::DeviceSize src_offset,
                    vk::DeviceSize dst_offset,
                    vk::DeviceSize size) const;

    void createImage(
        uint32_t width,
        uint32_t height,
        unsigned mip_levels,
        vk::SampleCountFlagBits samples,
        vk::Format format,
        vk::ImageTiling tiling,
        vk::ImageUsageFlags image_usage_flags,
        vk::MemoryPropertyFlags memory_property_flags,
        vk::raii::Image &image,
        vk::raii::DeviceMemory &image_memory) const;

    [[nodiscard]] vk::SampleCountFlagBits getUsableSampleCounts() const;

    [[nodiscard]] std::unique_ptr<vk::raii::CommandBuffer> beginTransientCommandBuffer() const;

    void endTransientCommandBuffer(const vk::raii::CommandBuffer &command_buffer) const;
};

