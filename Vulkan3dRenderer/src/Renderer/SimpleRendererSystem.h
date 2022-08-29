#pragma once

#include "Renderer/Device.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Renderer.h"

#include <memory>
#include <vector>

#include "GameObject.h"
#include "Camera.h"

namespace Vipera
{
	struct FrameInfo;

	class SimpleRendererSystem
	{
	public:
		SimpleRendererSystem(Device& device, VkRenderPass renderPass);
		~SimpleRendererSystem();

		SimpleRendererSystem(const SimpleRendererSystem&) = delete;
		SimpleRendererSystem& operator=(const SimpleRendererSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo,
			std::vector<GameObject>& gameObjects);

	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);
		
		
		Device& m_Device;

		std::unique_ptr<Pipeline> m_Pipeline;
		VkPipelineLayout pipelineLayout;
	};
}


