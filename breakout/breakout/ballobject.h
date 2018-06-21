#ifndef BALLOBJECT_H
#define BALLOBJECT_H

#include "gameobject.h"
#include "spriteRender.h"
#include "texture.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class BallObject : public GameObject
{
public:
	GLfloat Radius;
	GLboolean Stuck;
	GLboolean Sticky, PassThrough;

	BallObject();
	BallObject(glm::vec2 pos, GLfloat radius, glm::vec2 velocity, Texture2D sprite);

	glm::vec2 Move(GLfloat dt, GLuint window_width);
	void Reset(glm::vec2 position, glm::vec2 velocity);
};

#endif
