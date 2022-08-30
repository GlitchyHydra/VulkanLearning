#pragma once

#include "GameObject.h"

namespace Vipera
{
	typedef struct VkCommandBuffer_T* VkCommandBuffer;
	class Camera;

	struct FrameInfo
	{
		int FrameIndex;
		float FrameTime;
		VkCommandBuffer CommandBuffer;
		Camera& Camera;
		VkDescriptorSet globalDescriptorSet;
		GameObject::Map& GameObjects;
	};
}
