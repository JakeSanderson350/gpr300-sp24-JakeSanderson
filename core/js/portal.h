#pragma once
#include "../ew/external/glad.h"

#include "../ew/model.h"
#include "../ew/transform.h"
#include "../ew/shader.h"
#include "../ew/procGen.h"
#include "../ew/camera.h"
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
			void SetColor(glm::vec3 _color);
			glm::vec3 GetColor();
			void setTransform(ew::Transform t);
			ew::Transform getTransform();

			glm::mat4 const ClippedProjMat(glm::mat4 const &viewMat, glm::mat4 const &projMat);

			void draw(glm::mat4 const& viewMat, glm::mat4 const& projMat, ew::Shader portalShader);
			ew::Transform transform;

			void initBuffers();

		private:
			
			ew::Camera portalCamera;
			ew::Mesh portalPlane;
			Portal* destination;
			glm::vec3 normal;
			glm::vec3 color;
			
			GLuint p_vao;
			GLuint p_vbo;
			float portalVertices[18] = {
				-1.0f, -1.4f, 0.0f,
				1.0f, -1.4f, 0.0f,
				1.0f, 1.4f, 0.0f,

				1.0f, 1.4f, 0.0f,
				-1.0f,  1.4f, 0.0f,
				-1.0f, -1.4f, 0.0f,
			};
	};
}