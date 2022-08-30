#pragma once

#include "Camera.h"
#include "Renderer/Device.h"
#include "Renderer/FrameInfo.h"
#include "GameObject.h"
#include "Renderer/Pipeline.h"

// std
#include <memory>
#include <vector>

namespace Vipera
{
    class PointLightSystem
    {
    public:
        PointLightSystem(
            Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem&) = delete;
        PointLightSystem& operator=(const PointLightSystem&) = delete;

        void render(FrameInfo& frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        Device& m_Device;

        std::unique_ptr<Pipeline> m_Pipeline;
        VkPipelineLayout pipelineLayout;
    };
}  // namespace lve