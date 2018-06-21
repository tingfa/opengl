#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "texture.h"
#include "spriteRender.h"
#include "shader.h"

class PostProcessor
{
public:
	Shader PostProcessingShader;
	Texture2D Texture;
	GLuint Width, Height;
	GLboolean Chaos, Confuse, Shake;
	PostProcessor(Shader shader, GLuint width, GLuint height, GLboolean chaos, GLboolean confuse, GLboolean shake);
	void BeginRender();
	void EndRender();
	void Render(GLfloat time);
private:
	GLuint MSFBO, FBO;
	GLuint RBO;
	GLuint VAO;
	void initRenderData();
};

#endif
