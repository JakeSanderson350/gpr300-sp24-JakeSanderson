#include "portal.h"

namespace js
{
	//Constructors/destructors
	Portal::Portal()
	{
		portalPlane = ew::createPlane(10, 20, 10);
		destination = nullptr;
		normal = glm::vec3(1, 0, 0);
	}
	Portal::Portal( Portal* peartal)
	{
		portalPlane = ew::createPlane(10, 20, 10);
		destination = peartal;
		peartal->SetDestination(this);
		normal = glm::vec3(1, 0, 0);
	}
	Portal::~Portal()
	{
		destination = nullptr;
		delete(destination);
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
}