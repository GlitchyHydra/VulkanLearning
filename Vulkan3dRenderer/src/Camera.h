#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

namespace Vipera
{
	class Camera
	{
		glm::mat4 ProjectionMatrix{ 1.f };
		glm::mat4 ViewMatrix{ 1.f };

	public:

		void SetOrthographicsProjection(float left, float right, float bottom, float top, float near, float far);
		void SetPerspectiveProjection(float fovy, float aspect, float near, float far);

		void SetViewDirection(glm::vec3 pos, glm::vec3 direction, glm::vec3 up = {0.f, -1.f, 0.f});
		void SetViewTarget(glm::vec3 pos, glm::vec3 target, glm::vec3 up = { 0.f, -1.f, 0.f });
		void SetViewYXZ(glm::vec3 pos, glm::vec3 rotation);

		const glm::mat4& GetProjectionMatrix() const { return ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return ViewMatrix; }
	private:

	};
}

