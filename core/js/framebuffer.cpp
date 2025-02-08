#include "framebuffer.h"

namespace js
{
	Framebuffer createFramebuffer(int _width, int _height)
	{
		Framebuffer framebuffer;

		glGenFramebuffers(1, &framebuffer.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

		// color attachment
		glGenTextures(1, &framebuffer.color0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.color0, 0);

		// Other color attach for HDR
		glGenTextures(1, &framebuffer.color1);
		glBindTexture(GL_TEXTURE_2D, framebuffer.color1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); //Changed from GL_RGB to GL_RGB16F
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebuffer.color1, 0);

		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);

		//check completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			//std::cout << "Framebuffer incomplete : % d";
			return framebuffer;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return framebuffer;
	}
}