#pragma once

#include "Renderer/Device.h"
#include "Renderer/Renderer.h"
#include "Window.h"

// std
#include <memory>
#include <vector>

namespace Vipera
{
	class GameObject;

	class Application
	{
	 public:
	  static constexpr int WIDTH = 800;
	  static constexpr int HEIGHT = 600;
	
	  Application();
	  ~Application();
	
	  Application(const Application&) = delete;
	  Application&operator=(const Application&) = delete;
	
	  void run();
	
	 private:
	  void load2dGameObjects();
	  void load3dGameObjects();
	
	  Window m_Window{WIDTH, HEIGHT, "Vulkan Tutorial"};
	  Device m_Device{ m_Window };
	  Renderer m_Renderer{m_Window, m_Device};
	  
	  std::vector<GameObject> m_GameObjects;
	};
}  // namespace lve
