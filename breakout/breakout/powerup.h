#ifndef POWERUP_H
#define POWERUP_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "texture.h"
#include "gameobject.h"
#include <string>

const glm::vec2 SIZEP(60, 20);
const glm::vec2 VELOCITY(0.0f, 150.0f);

class PowerUp : public GameObject
{
public:
	std::string Type;
	GLfloat Duration;
	GLboolean Activated;

	PowerUp(std::string type, glm::vec3 color, GLfloat duration, glm::vec2 position, Texture2D texture)
		: GameObject(position, SIZEP, texture, color, VELOCITY), Type(type), Duration(duration), Activated()
	{

	}
};

#endif
