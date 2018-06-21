#ifndef GAMELEVEL_H
#define GAMELEVEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

#include "gameobject.h"
#include "spriteRender.h"
#include "resource_manager.h"

class GameLevel
{
public:
	std::vector<GameObject> Bricks;

	GameLevel(){}
	void Load(const GLchar* file, GLuint levelWidth, GLuint levelHeight);
	void Draw(spriteRender& renderer);
	GLboolean IsCompleted();

private:
	void init(std::vector<std::vector<GLuint>> tileData, GLuint levelWidth, GLuint levelHeight);
};

#endif
