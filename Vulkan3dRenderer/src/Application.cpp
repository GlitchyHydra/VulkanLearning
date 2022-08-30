#include "Application.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


// std
#include <array>
#include <cassert>
#include <stdexcept>

#include <chrono>

//Renderer
#include "Renderer/Systems/SimpleRendererSystem.h"
#include "Renderer/Systems/PointLightSystem.h"
#include "Renderer/Buffer.h"
#include "Renderer/FrameInfo.h"
#include "Renderer/Descriptors.h"

//Engine
#include "GameObject.h"
#include "Camera.h"
#include "ShortcutController.h"
#include "Log.h"

namespace Vipera
{
	struct GlobalUBO
	{
		glm::mat4 Projection{ 1.f };
		glm::mat4 View{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };  // w is intensity
		glm::vec3 lightPosition{ -1.f };
		alignas(16) glm::vec4 lightColor{ 1.f };  // w is light intensity
	};

	Application::Application()
	{
		globalPool =
			DescriptorPool::Builder(m_Device)
			.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		load3dGameObjects();
	}

	Application::~Application()
	{
	}

	void Application::run()
	{
		std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++)
		{
			uboBuffers[i] = std::make_unique<Buffer>(
				m_Device,
				sizeof(GlobalUBO),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}
		
		auto globalSetLayout = DescriptorSetLayout::Builder(m_Device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++)
		{
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			DescriptorWriter(*globalSetLayout, *globalPool)
			.writeBuffer(0, &bufferInfo)
			.build(globalDescriptorSets[i]);
		}

		SimpleRendererSystem simpleRenderSystem
		{
			m_Device,
			m_Renderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout()
		};
		PointLightSystem pointLightSystem
		{
			m_Device,
			m_Renderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout()
		};

		Camera camera{};

		auto viewerObject = GameObject::CreateGameObject();
		viewerObject.transform.translation.z = -2.5f;
		ShortcutController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();
		while (!m_Window.shouldClose())
		{
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime =
				std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			cameraController.moveInPlaneXZ(m_Window.GetWindow(), frameTime, viewerObject);
			camera.SetViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			float aspect = m_Renderer.GetAspectRatio();
			camera.SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);

			if (auto commandBuffer = m_Renderer.beginFrame())
			{
				int frameIndex = m_Renderer.gerFrameIndex();
				FrameInfo frameInfo{ frameIndex, frameTime,
					commandBuffer, camera,
					globalDescriptorSets[frameIndex],
				m_GameObjects};

				GlobalUBO ubo{};
				ubo.Projection = camera.GetProjectionMatrix();
				ubo.View = camera.GetViewMatrix();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				m_Renderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo);
				pointLightSystem.render(frameInfo);
				m_Renderer.endSwapChainRenderPass(commandBuffer);
				m_Renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(m_Device.device());
	}

	void Application::load2dGameObjects()
	{
		/*std::vector<Model::Vertex> vertices
		{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};
		auto model = std::make_shared<Model>(m_Device, vertices);

		auto triangle = GameObject::CreateGameObject();
		triangle.model = model;
		triangle.color = { .1f, .8f, .1f };
		triangle.transform2d.translation.x = .2f;
		triangle.transform2d.scale = { 2.f, .5f };
		triangle.transform2d.rotation = .25f * glm::two_pi<float>();

		m_GameObjects.push_back(std::move(triangle));*/
	}

	// temporary helper function, creates a 1x1x1 cube centered at offset
	std::unique_ptr<Model> createCubeModel(Device& device, glm::vec3 offset)
	{
		Model::Builder modelBuilder{};
		modelBuilder.vertices = {
			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
		};
		for (auto& v : modelBuilder.vertices)
		{
			v.position += offset;
		}

		modelBuilder.indices = { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
								12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

		return std::make_unique<Model>(device, modelBuilder);
	}

	void Application::load3dGameObjects()
	{
		std::shared_ptr<Model> modelFlat = Model::CreateModelFromFile(m_Device, "models/flat_vase.obj");
		auto flatVase = GameObject::CreateGameObject();
		flatVase.model = modelFlat;
		flatVase.transform.translation = { -.5f, .5f, 0.f };
		flatVase.transform.scale = { 3.f, 1.5f, 3.f };
		//cube.transform.rotation = { 0.f, 0.f, 0.f };
		m_GameObjects.emplace(flatVase.getId(), std::move(flatVase));

		std::shared_ptr<Model> modelSmooth = Model::CreateModelFromFile(m_Device, "models/smooth_vase.obj");
		auto smoothVase = GameObject::CreateGameObject();
		smoothVase.model = modelSmooth;
		smoothVase.transform.translation = { .5f, .5f, 0.f };
		smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
		m_GameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

		std::shared_ptr<Model> modelPlane = Model::CreateModelFromFile(m_Device, "models/quad.obj");
		auto plane = GameObject::CreateGameObject();
		plane.model = modelPlane;
		plane.transform.translation = { .0f, .5f, 0.f };
		plane.transform.scale = { 3.f, 1.5f, 3.f };
		m_GameObjects.emplace(plane.getId(), std::move(plane));
	}
	
} // namespace 
