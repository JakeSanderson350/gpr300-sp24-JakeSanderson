#pragma once
#include "../ew/external/glad.h"
#include <GLFW/glfw3.h>

namespace js
{
	struct Framebuffer
	{
		GLuint fbo;
		GLuint color0;
		GLuint color1;
		GLuint depth;

		/*Framebuffer()
		{
			fbo = 0;
			color0 = 0;
			color1 = 0;
			depth = 0;
		}*/
	};

	Framebuffer createFramebuffer(int _width, int _height);
}