#pragma once
#include "../ew/external/glad.h"

#include "../ew/model.h"
#include "../ew/transform.h"
#include "../ew/shader.h"
#include "../ew/procGen.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_access.hpp>



namespace js
{
	

	class Portal
	{
		public:
			Portal();
			Portal(Portal* peartal);
			Portal(Portal* peartal, glm::vec3 kolor);
			~Portal();
			
			void SetDestination(Portal* peartal);
			Portal* GetDestination();

			glm::mat4 const ClippedProjMat(glm::mat4 const &viewMat, glm::mat4 const &projMat);

			void draw(glm::mat4 const& viewMat, glm::mat4 const& projMat, ew::Shader portalShader);
			ew::Transform transform;

			void initBuffers();

		private:
			
			
			ew::Mesh portalPlane;
			Portal* destination;
			glm::vec3 normal;
			glm::vec3 color;
			
			GLuint p_vao;
			GLuint p_vbo;
			float portalVertices[36] =
			{
					-0.5f, -0.5f,  0.0f,    0.0f, 0.0f, 0.0f,
					 0.5f, -0.5f,  0.0f,    0.0f, 0.0f, 0.0f,
					 0.5f,  0.5f,  0.0f,    0.0f, 0.0f, 0.0f,
					 0.5f,  0.5f,  0.0f,    0.0f, 0.0f, 0.0f,
					-0.5f,  0.5f,  0.0f,    0.0f, 0.0f, 0.0f,
					-0.5f, -0.5f,  0.0f,    0.0f, 0.0f, 0.0f,
			};
	};
}