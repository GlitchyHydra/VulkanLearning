#include "SimpleRendererSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>

#include "FrameInfo.h"

namespace Vipera
{
	struct SimplePushConstantData
	{
		glm::mat4 transform{ 1.f };
		glm::mat4 normalMatrix {1.f};
	};

	SimpleRendererSystem::SimpleRendererSystem(Device& device, VkRenderPass renderPass)
		:m_Device{device}
	{
		createPipelineLayout();
		createPipeline(renderPass);
	}

	SimpleRendererSystem::~SimpleRendererSystem()
	{
		vkDestroyPipelineLayout(m_Device.device(), pipelineLayout, nullptr);
	}
	
	void SimpleRendererSystem::createPipelineLayout()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void SimpleRendererSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		Pipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		m_Pipeline = std::make_unique<Pipeline>(
			m_Device,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig);
	}

	void SimpleRendererSystem::renderGameObjects(FrameInfo& frameInfo,
		std::vector<GameObject>& gameObjects)
	{
		m_Pipeline->bind(frameInfo.CommandBuffer);

		auto projectionView = 
			frameInfo.Camera.GetProjectionMatrix() * frameInfo.Camera.GetViewMatrix();

		for (auto& gameObj : gameObjects)
		{
			auto modelMatrix = gameObj.transform.mat4();
			SimplePushConstantData push
			{
				  projectionView * modelMatrix,
				gameObj.transform.normalMatrix(),
			};

			vkCmdPushConstants(frameInfo.CommandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);
			gameObj.model->bind(frameInfo.CommandBuffer);
			gameObj.model->draw(frameInfo.CommandBuffer);
		}
	}
}