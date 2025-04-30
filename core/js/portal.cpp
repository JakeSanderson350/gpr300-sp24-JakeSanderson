#include "portal.h"
#include <iostream>

namespace js
{
	//Constructors/destructors
	Portal::Portal()
	{
		//portalPlane = ew::createPlane(10, 20, 10);
		destination = nullptr;
		normal = glm::vec3(1, 0, 0);
		color = glm::vec3(1, 0, 1);
		transform = ew::Transform();
	}
	Portal::Portal(Portal* peartal)
	{
		//portalPlane = ew::createPlane(10, 20, 10);
		destination = peartal;
		peartal->SetDestination(this);
		normal = glm::vec3(1, 0, 0);
		color = glm::vec3(1, 0, 1);
		transform = ew::Transform();
	}
	Portal::Portal(Portal* peartal, glm::vec3 kolor)
	{
		//portalPlane = ew::createPlane(10, 20, 10);
		destination = peartal;
		peartal->SetDestination(this);
		normal = glm::vec3(1, 0, 0);
		color = kolor;
		transform = ew::Transform();
	}
	Portal::~Portal()
	{
		destination = nullptr;
		delete(destination);
	}

	//function init buffers
	void Portal::initBuffers()
	{
		glGenVertexArrays(1, &p_vao);
		glGenBuffers(1, &p_vbo);
		glBindVertexArray(p_vao);
		glBindBuffer(GL_ARRAY_BUFFER, p_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(portalVertices), &portalVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		portalPlane = ew::createPlane(5, 10, 10);
	}

	//Getters and setters
	void Portal::SetDestination(Portal* peartal)
	{
		destination = peartal;
	}
	Portal* Portal::GetDestination()
	{
		return destination;
	}
	void Portal::SetColor(glm::vec3 _color)
	{
		color = _color;
	}
	glm::vec3 Portal::GetColor()
	{
		return color;
	}

	//function to get the new clipped projection matrix using oblique view frustum near plane clipping technique
	glm::mat4 const Portal::ClippedProjMat(glm::mat4 const &viewMat, glm::mat4 const &projMat, ew::Camera const &camera)
	{
		//float distance = glm::length(transform.position);
		//glm::vec3 newClipPlaneNormal = transform.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
		////Calculate the clip plane using the new clip plane's normal and its distance to the origin
		//glm::vec4 newClipPlane(newClipPlaneNormal, distance);
		//newClipPlane = glm::inverse(glm::transpose(viewMat)) * newClipPlane;
		////If the new clip plane is facing away from the camera, use the old projection matrix
		//if (newClipPlane.w > 0.0f)
		////{
		////	return projMat;
		////}
		//glm::vec4 c = glm::inverse(projMat) * glm::vec4(glm::sign(newClipPlane.x), glm::sign(newClipPlane.y), 1.0f, 1.0f);
		//glm::mat4 newProjMat = projMat;
		//newProjMat = glm::row(newProjMat, 2, c - glm::row(newProjMat, 3));

		//return newProjMat;

		//float d = glm::length(transform.position);
		glm::vec3 newCLipPlaneNormal = transform.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
		float d = glm::dot(-newCLipPlaneNormal, glm::normalize(camera.position - transform.position));
		std::cout << d << "\n";

		// Calculate the clip plane with a normal and distance to the origin
		glm::vec4 newClipPlane(newCLipPlaneNormal, 0);
		newClipPlane = glm::inverse(glm::transpose(viewMat)) * newClipPlane;

		// If the new clip plane's fourth component (w) is greater than 0, indicating that it is facing away from the camera,
		if (newClipPlane.w > 0.0f)
		{
			//std::cout << "Balls ";
			return projMat;
		}

		glm::vec4 q = glm::inverse(projMat) * glm::vec4(
			glm::sign(newClipPlane.x),
			glm::sign(newClipPlane.y),
			1.0f,
			1.0f
		);

		glm::vec4 c = newClipPlane * (2.0f / (glm::dot(newClipPlane, q)));

		glm::mat4 newProjMat = projMat;
		// third row = new clip plane - fourth row of projection matrix
		newProjMat = glm::row(newProjMat, 2, c - glm::row(newProjMat, 3));

		return newProjMat;
	}
	
	void Portal::draw(glm::mat4 const& viewMat, glm::mat4 const& projMat, ew::Shader portalShader)
	{
		portalShader.use();
		//set uniforms
		portalShader.setMat4("_TransformModel", transform.modelMatrix());
		//portalShader.setMat4("_CameraViewproj", portalCamera.projectionMatrix() * portalCamera.viewMatrix());
		portalShader.setMat4("_CameraViewproj", projMat * viewMat);
		portalShader.setVec3("_PortalColor", color);
		
		/*ew::Transform planeTransform = transform;
		glm::rotate(planeTransform.rotation, 90.0f, glm::vec3(1, 0, 0));
		portalShader.setMat4("_TransformModel", planeTransform.modelMatrix());
		portalPlane.draw();*/

		glBindVertexArray(p_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

}