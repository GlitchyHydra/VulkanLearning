//
// Created by Kvan Valerii on 24.12.2025.
//

#include "RendererBase.h"

#include "Device.h"
#include "SwapChain.h"

namespace Vipera
{

    bool RendererBase::createUniformBuffers(Device& device, size_t uniformDataSize) {
        m_UniformBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < m_UniformBuffers.size(); i++)
        {
            m_UniformBuffers[i] = std::make_unique<Buffer>(
                device,
                sizeof(GlobalUBO),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            m_UniformBuffers[i]->map();
        }
        return true;
    }


}
