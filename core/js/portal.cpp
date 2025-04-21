#include "portal.h"

namespace js
{
	//Constructors/destructors
	Portal::Portal()
	{
		//portalPlane = ew::createPlane(10, 20, 10);
		destination = nullptr;
		normal = glm::vec3(1, 0, 0);
		color = glm::vec3(1, 0, 1);
	}
	Portal::Portal(Portal* peartal)
	{
		//portalPlane = ew::createPlane(10, 20, 10);
		destination = peartal;
		peartal->SetDestination(this);
		normal = glm::vec3(1, 0, 0);
		color = glm::vec3(1, 0, 1);
	}
	Portal::Portal(Portal* peartal, glm::vec3 kolor)
	{
		//portalPlane = ew::createPlane(10, 20, 10);
		destination = peartal;
		peartal->SetDestination(this);
		normal = glm::vec3(1, 0, 0);
		color = kolor;
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

	//function to get the new clipped projection matrix using oblique view frustum near plane clipping technique
	glm::mat4 const Portal::ClippedProjMat(glm::mat4 const &viewMat, glm::mat4 const &projMat)
	{
		float distance = glm::length(transform.position);
		glm::vec3 newClipPlaneNormal = transform.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
		//Calculate the clip plane using the new clip plane's normal and its distance to the origin
		glm::vec4 newClipPlane(newClipPlaneNormal, distance);
		newClipPlane = glm::inverse(glm::transpose(viewMat)) * newClipPlane;
		//If the new clip plane is facing away from the camera, use the old projection matrix
		if (newClipPlane.w > 0.0f)
		{
			return projMat;
		}
		glm::vec4 c = glm::inverse(projMat) * glm::vec4(glm::sign(newClipPlane.x), glm::sign(newClipPlane.y), 1.0f, 1.0f);
		glm::mat4 newProjMat = projMat;
		newProjMat = glm::row(newProjMat, 2, c - glm::row(newProjMat, 3));

		return glm::mat4();
	}

	void Portal::draw(glm::mat4 const& viewMat, glm::mat4 const& projMat, ew::Shader portalShader)
	{
		portalShader.use();
		//set uniforms
		portalShader.setMat4("_TransformModel", transform.modelMatrix());
		portalShader.setMat4("_CameraViewproj", ClippedProjMat(viewMat, projMat));
		portalShader.setVec3("_PortalColor", color);
		//portalPlane.draw();

		glBindVertexArray(p_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

}