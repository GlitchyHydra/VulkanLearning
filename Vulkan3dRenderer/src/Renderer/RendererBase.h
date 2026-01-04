//
// Created by Kvan Valerii on 24.12.2025.
//

#ifndef VULKAN3DRENDERER_RENDERERBASE_H
#define VULKAN3DRENDERER_RENDERERBASE_H

#include "Device.h"

namespace Vipera {

    class Buffer;

    class RendererBase {

    public:
        explicit RendererBase(const VulkanRenderDevice& vkDev, VulkanImage depthTexture)
        : device_(vkDev.device)
        , framebufferWidth_(vkDev.framebufferWidth)
        , framebufferHeight_(vkDev.framebufferHeight)
        , depthTexture_(depthTexture)
        {}
        virtual ~RendererBase();
        virtual void fillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage) = 0;

        inline VulkanImage getDepthTexture() const { return depthTexture_; }

    protected:
        void beginRenderPass(VkCommandBuffer commandBuffer, size_t currentImage);
        bool createUniformBuffers(Device& device, size_t uniformDataSize);

        VkDevice device_ = nullptr;

        uint32_t framebufferWidth_ = 0;
        uint32_t framebufferHeight_ = 0;

        // Depth buffer
        VulkanImage depthTexture_;

        // Descriptor set (layout + pool + sets) -> uses uniform buffers, textures, framebuffers
        VkDescriptorSetLayout descriptorSetLayout_ = nullptr;
        VkDescriptorPool descriptorPool_ = nullptr;
        std::vector<VkDescriptorSet> descriptorSets_;

        // Framebuffers (one for each command buffer)
        std::vector<VkFramebuffer> swapchainFramebuffers_;

        // 4. Pipeline & render pass (using DescriptorSets & pipeline state options)
        VkRenderPass renderPass_ = nullptr;
        VkPipelineLayout pipelineLayout_ = nullptr;
        VkPipeline graphicsPipeline_ = nullptr;

        // 5. Uniform buffer
        std::vector<std::unique_ptr<Buffer>> m_UniformBuffers;
        std::vector<VkDeviceMemory> m_UniformBuffersMemory;
    };
}

#endif //VULKAN3DRENDERER_RENDERERBASE_H