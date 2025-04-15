#pragma once
#include "../ew/external/glad.h"

#include "../ew/model.h"
#include "../ew/procGen.h"
#include <GLFW/glfw3.h>

namespace js
{
	class Portal
	{
		public:
			Portal();
			Portal( Portal* peartal);
			~Portal();

			void SetDestination(Portal* peartal);
			Portal* GetDestination();

			glm::mat4 ClippedProjMat(glm::mat4 &viewMat, glm::mat4 &projMat);

			void draw();

		private:
			ew::Transform transform;
			ew::Mesh portalPlane;
			Portal* destination;
			glm::vec3 normal;
	};
}