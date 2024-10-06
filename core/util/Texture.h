#pragma once

#include <map>

#ifdef EMSCRIPTEN
#include <GLFW/emscripten_glfw3.h>
#include <GLES3/gl3.h>
#else
#include "../ew/external/glad.h"
#endif

#include "../ew/external/stb_image.h"

const auto COMPONENTS_TO_FORMAT = std::map<int, GLenum>{
	{1, GL_RED},
	{2, GL_RG},
	{3, GL_RGB},
	{4, GL_RGBA},
};

const auto COMPONENTS_TO_INTERNAL_FORMAT = std::map<int, GLenum>{
	{1, GL_R8},
	{2, GL_RG},
	{3, GL_RGB},
	{4, GL_RGBA},
};

namespace Util
{
	GLuint loadTexture(const char* filepath, GLint wrapMode = GL_CLAMP_TO_EDGE, GLint filtering = GL_LINEAR, bool flipVertical = true);
}