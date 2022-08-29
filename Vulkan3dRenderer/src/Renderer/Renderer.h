#pragma once

#include "Window.h"
#include "Device.h"
#include "SwapChain.h"

#include <cassert>
#include <vector>
#include <memory>

namespace Vipera
{
	class Renderer
	{
		Window& m_Window;
		Device& m_Device;
		std::unique_ptr<SwapChain> m_SwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex {0};
		bool isFrameStarted{false};

	public:
		Renderer(Window& window, Device& device);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const { return m_SwapChain->getRenderPass(); }
		float GetAspectRatio() { return m_SwapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const
		{
			assert(isFrameStarted && "Cannot get command buffer when frame is not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int gerFrameIndex() const
		{
			assert(isFrameStarted && "");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
	};
}
