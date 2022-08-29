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

#include "Renderer/SimpleRendererSystem.h"
#include "Renderer/Buffer.h"

#include "GameObject.h"
#include "Camera.h"
#include "ShortcutController.h"
#include "Renderer/FrameInfo.h"


namespace Vipera
{
	struct GlobalUBO
	{
		glm::mat4 ProjectionView{ 1.f };
		glm::vec3 LightDirection = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
	};

	Application::Application()
	{
		load3dGameObjects();
	}

	Application::~Application()
	{
	}

	void Application::run()
	{
		Buffer globalUboBuffer
		{
			m_Device,
			sizeof(GlobalUBO),
			SwapChain::MAX_FRAMES_IN_FLIGHT,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			m_Device.properties.limits.minUniformBufferOffsetAlignment
		};
		globalUboBuffer.map();

		SimpleRendererSystem simpleRenderSystem{ m_Device, m_Renderer.getSwapChainRenderPass() };
		Camera camera{};

		auto viewerObject = GameObject::CreateGameObject();
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
			camera.SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

			if (auto commandBuffer = m_Renderer.beginFrame())
			{
				int frameIndex = m_Renderer.gerFrameIndex();
				FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, camera };

				GlobalUBO ubo{};
				ubo.ProjectionView = camera.GetProjectionMatrix() * camera.GetViewMatrix();
				globalUboBuffer.writeToIndex(&ubo, frameIndex);
				globalUboBuffer.flushIndex(frameIndex);

				m_Renderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo, m_GameObjects);
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
		//std::shared_ptr<Model> model = createCubeModel(m_Device, {0.f, 0.f, 0.f});
		//std::shared_ptr<Model> model = Model::CreateModelFromFile(m_Device, "models/colored_cube.obj");
		std::shared_ptr<Model> modelFlat = Model::CreateModelFromFile(m_Device, "models/flat_vase.obj");

		auto flatVase = GameObject::CreateGameObject();
		flatVase.model = modelFlat;
		flatVase.transform.translation = { -.5f, .5f, 2.5f };
		flatVase.transform.scale = { 3.f, 1.5f, 3.f };
		//cube.transform.rotation = { 0.f, 0.f, 0.f };
		m_GameObjects.push_back(std::move(flatVase));

		std::shared_ptr<Model> modelSmooth = Model::CreateModelFromFile(m_Device, "models/smooth_vase.obj");
		auto smoothVase = GameObject::CreateGameObject();
		smoothVase.model = modelSmooth;
		smoothVase.transform.translation = { .5f, .5f, 2.5f };
		smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
		//cube.transform.rotation = { 0.f, 0.f, 0.f };
		m_GameObjects.push_back(std::move(smoothVase));
	}

	
	
} // namespace 
