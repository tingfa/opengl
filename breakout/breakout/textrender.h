#ifndef TEXTRENDER_H
#define TEXTRENDER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <map>
#include <string>
#include "texture.h"
#include "shader.h"

struct Character
{
	GLuint TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	GLuint Advance;
};

class TextRender
{
public:
	std::map<GLchar, Character> Characters;
	Shader TextShader;
	TextRender(GLuint width, GLuint height);
	void Load(std::string font, GLuint fontSize);
	void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color = glm::vec3(1.0f));
private:
	GLuint VAO, VBO;
};

#endif
