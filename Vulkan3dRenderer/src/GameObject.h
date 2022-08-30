#pragma once

#include "Renderer/Model.h"

#include <unordered_map>
#include <memory>

#include <glm/gtc/matrix_transform.hpp>

namespace Vipera
{
	struct Transform2dComponent
	{
		glm::vec2 translation{};
		glm::vec2 scale{ 1.f,1.f };
		float rotation;

		glm::mat2 mat2()
		{
			const float s = glm::sin(rotation);
			const float c = glm::cos(rotation);
			glm::mat2 rotMatrix
			{
				{c, s},
				{-s, c}
			};

			glm::mat2 scaleMat
			{
				{scale.x, .0f},
				{.0f, scale.y}
			};
			return rotMatrix * scaleMat;
		}
	};

	struct Transform3dComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};

		// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};

	class GameObject
	{
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, GameObject>;

		static GameObject CreateGameObject()
		{
			static id_t currentId = 0;
			return GameObject{ currentId++ };
		}

		GameObject(const GameObject&) = delete;
		GameObject& operator=(const GameObject&) = delete;

		GameObject(GameObject&&) = default;
		GameObject& operator=(GameObject&&) = default;

		id_t getId() { return id; }

		std::shared_ptr<Model> model{};
		glm::vec3 color{};
		Transform3dComponent transform;

	private:
		GameObject(id_t objId) : id{ objId } {}

		id_t id;
	};
}
