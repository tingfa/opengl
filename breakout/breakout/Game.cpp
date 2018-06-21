#include "game.h"
#include "resource_manager.h"
#include "spriteRender.h"
#include "ballobject.h"
#include "particle_generator.h"
#include "postprocessor.h"
#include "textrender.h"
#include <glm/gtc/matrix_transform.hpp>
#include <irrklang/irrKlang.h>
#include <algorithm>
#include <sstream>
using namespace std;
using namespace irrklang;

spriteRender* Renderer;
GameObject *Player;	
BallObject *Ball;
ParticleGenerator *Particles;
PostProcessor* Effects;
GLfloat ShakeTime = 0.0f;
ISoundEngine *SoundEngine = createIrrKlangDevice();
TextRender* Text;

Game::Game(GLuint width, GLuint height) : State(GAME_ACTIVE), Width(width), Height(height), Lives(3)
{

}

Game::~Game()
{
	delete Renderer;
	delete Player;
	delete Ball;
	delete Particles;
	delete Effects;
	delete Text;
}

void Game::Init()
{
	ResourceManager::LoadShader("spriteshader.vs", "spriteshader.fs", nullptr, "sprite");
	ResourceManager::LoadShader("particle.vs", "particle.fs", nullptr, "particle");
	ResourceManager::LoadShader("postprocess.vs", "postprocess.fs", nullptr, "postprocess");
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height),
		0.0f, -1.0f, 1.0f);

	ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("particle").SetMatrix4("projection", projection);

	Renderer = new spriteRender(ResourceManager::GetShader("sprite"));
	ResourceManager::LoadTexture("background.jpg", GL_FALSE, "background");
	ResourceManager::LoadTexture("awesomeface.png", GL_TRUE, "face");
	ResourceManager::LoadTexture("block.png", GL_FALSE, "block");
	ResourceManager::LoadTexture("block_solid.png", GL_FALSE, "block_solid");
	ResourceManager::LoadTexture("paddle.png", GL_TRUE, "paddle");
	ResourceManager::LoadTexture("particle.png", GL_TRUE, "particle");
	ResourceManager::LoadTexture("powerup_speed.png", GL_TRUE, "powerup_speed");
	ResourceManager::LoadTexture("powerup_sticky.png", GL_TRUE, "powerup_sticky");
	ResourceManager::LoadTexture("powerup_passthrough.png", GL_TRUE, "powerup_passthrough");
	ResourceManager::LoadTexture("powerup_increase.png", GL_TRUE, "powerup_increase");
	ResourceManager::LoadTexture("powerup_chaos.png", GL_TRUE, "powerup_chaos");
	ResourceManager::LoadTexture("powerup_confuse.png", GL_TRUE, "powerup_confuse");

	GameLevel one;
	one.Load("one.lvl", this->Width, this->Height * 0.5);
	GameLevel two;
	two.Load("two.lvl", this->Width, this->Height * 0.5);
	GameLevel three;
	three.Load("three.lvl", this->Width, this->Height * 0.5);
	GameLevel four;
	four.Load("four.lvl", this->Width, this->Height * 0.5);

	this->levels.push_back(one);
	this->levels.push_back(two);
	this->levels.push_back(three);
	this->levels.push_back(four);

	this->level = 1;

	glm::vec2 playerPos = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
	Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));

	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2);
	Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));

	Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 800);

	Effects = new PostProcessor(ResourceManager::GetShader("postprocess"), this->Width, this->Height, false, false, false);

	SoundEngine->play2D("breakout.mp3", GL_TRUE);

	Text = new TextRender(this->Width, this->Height);
	Text->Load("arial.ttf", 24);
}

void Game::ProcessInput(GLfloat dt)
{
	if (this->State == GAME_ACTIVE)
	{
		GLfloat velocity = PLAYER_VELOCITY * dt;
		if (this->Keys[GLFW_KEY_A])
		{
			if (Player->Position.x >= 0)
			{
				Player->Position.x -= velocity;
				if (Ball->Stuck)
				{
					Ball->Position.x -= velocity;
				}
			}
		}
		if (this->Keys[GLFW_KEY_D])
		{
			if (Player->Position.x <= this->Width - Player->Size.x)
			{
				Player->Position.x += velocity;
				if (Ball->Stuck)
				{
					Ball->Position.x += velocity;
				}
			}
		}
		if (this->Keys[GLFW_KEY_SPACE])
		{
			Ball->Stuck = false;
		}
	}
	if (this->State == GAME_MENU)
	{
		if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
		{
			this->State = GAME_ACTIVE;
			this->KeysProcessed[GLFW_KEY_ENTER] = GL_TRUE;
		}
		if (this->Keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W])
		{
			this->level = this->level % 4 + 1;
			this->KeysProcessed[GLFW_KEY_W] = GL_TRUE;
		}
		if (this->Keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S])
		{
			if (this->level > 1)
				--this->level;
			else
				this->level = 4;
			this->KeysProcessed[GLFW_KEY_S] = GL_TRUE;
		}
	}
	if (this->State == GAME_WIN)
	{
		if (this->Keys[GLFW_KEY_ENTER])
		{
			this->KeysProcessed[GLFW_KEY_ENTER] = GL_TRUE;
			Effects->Chaos = GL_FALSE;
			this->State = GAME_MENU;
		}
	}
}

void Game::Update(GLfloat dt)
{
	Ball->Move(dt, this->Width);
	this->DoCollisions();

	Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2));

	if (ShakeTime > 0.0f)
	{
		ShakeTime -= dt;
		if (ShakeTime <= 0.0f)
		{
			Effects->Shake = false;
		}
	}

	this->UpdatePowerUps(dt);

	if (Ball->Position.y >= this->Height)
	{
		--this->Lives;
		if (this->Lives == 0)
		{
			this->ResetLevel();
			this->State = GAME_MENU;
		}
		this->ResetPlayer();
	}

	if (this->State == GAME_ACTIVE && this->levels[this->level - 1].IsCompleted())
	{
		this->ResetLevel();
		this->ResetPlayer();
		Effects->Chaos = GL_TRUE;
		this->State = GAME_WIN;
	}
}

void Game::ResetLevel()
{
	if (this->level == 1)
	{
		this->levels[this->level - 1].Load("one.lvl", this->Width, this->Height * 0.5f);
	}
	else if (this->level == 2)
	{
		this->levels[this->level - 1].Load("two.lvl", this->Width, this->Height * 0.5f);
	}
	else if (this->level == 3)
	{
		this->levels[this->level - 1].Load("three.lvl", this->Width, this->Height * 0.5f);
	}
	else if (this->level == 4)
	{
		this->levels[this->level - 1].Load("four.lvl", this->Width, this->Height * 0.5f);
	}
	this->Lives = 3;
}

void Game::ResetPlayer()
{
	Player->Size = PLAYER_SIZE;
	Player->Position = glm::vec2(this->Width / 2 - Player->Size.x / 2, this->Height - PLAYER_SIZE.y);
	Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -(BALL_RADIUS * 2)), INITIAL_BALL_VELOCITY);
}

void Game::Render()
{
	if (this->State == GAME_ACTIVE || this->State == GAME_MENU || this->State == GAME_WIN)
	{
		Effects->BeginRender();
		Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f);
		this->levels[this->level - 1].Draw(*Renderer);
		Player->Draw(*Renderer);
		Particles->Draw();
		Ball->Draw(*Renderer);
		for (PowerUp &powerUp : this->PowerUps)
		{
			if (!powerUp.Destroyed)
			{
				powerUp.Draw(*Renderer);
			}
		}
		Effects->EndRender();
		Effects->Render(glfwGetTime());	

		stringstream ss;
		ss << this->Lives;
		Text->RenderText("Lives:" + ss.str(), 5.0f, 5.0f, 1.0f);
	}
	if (this->State == GAME_MENU)
	{
		Text->RenderText("Press ENTER to start", 300.0f, Height / 2, 1.0f);
		Text->RenderText("Press W or S to select level", 305.0f, Height / 2 + 30.0f, 0.75f);
	}
	if (this->State == GAME_WIN)
	{
		Text->RenderText(
			"You WON!!!", 320.0, Height / 2 - 20.0, 1.0, glm::vec3(0.0, 1.0, 0.0)
			);
		Text->RenderText(
			"Press ENTER to retry or ESC to quit", 130.0, Height / 2, 1.0, glm::vec3(1.0, 1.0, 0.0)
			);
	}
/*	Renderer->DrawSprite(ResourceManager::GetTexture("face"), glm::vec2(200, 200), glm::vec2(300, 400), 45.0f,
		glm::vec3(0.0f, 1.0f, 0.0f));*/
}

GLboolean checkCollision(GameObject& one, GameObject& two)
{
	bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
		two.Position.x + two.Size.x >= one.Position.x;
	bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
		two.Position.y + two.Size.y >= one.Position.y;
	return collisionX && collisionY;
}

Direction VectorDirection(glm::vec2 target)
{
	glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f),  // ио
		glm::vec2(1.0f, 0.0f),  // ср
		glm::vec2(0.0f, -1.0f), // об
		glm::vec2(-1.0f, 0.0f)  // вС
	};

	GLfloat max = 0.0f;
	GLuint best_match = -1;
	for (GLuint i = 0; i < 4; ++i)
	{
		GLfloat dot_product = glm::dot(glm::normalize(target), compass[i]);
		if (dot_product > max)
		{
			max = dot_product;
			best_match = i;
		}
	}
	return Direction(best_match);
}

Collision checkCollision(BallObject& one, GameObject& two)
{
	glm::vec2 center = glm::vec2(one.Position + one.Radius);
	glm::vec2 aabb_half_extents(two.Size.x / 2, two.Size.y / 2);
	glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x,
		two.Position.y + aabb_half_extents.y);

	glm::vec2 difference = center - aabb_center;
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);

	glm::vec2 closest = aabb_center + clamped;
	difference = closest - center;

//	return glm::length(difference) < one.Radius;
	if (glm::length(difference) <= one.Radius)
	{
		return make_tuple(GL_TRUE, VectorDirection(difference), difference);
	}
	else
	{
		return make_tuple(GL_FALSE, UP, glm::vec2(0, 0));
	}
}

/*void Game::DoCollisions()
{
	for (GameObject &box : this->levels[this->level - 1].Bricks)
	{
		if (!box.Destroyed)
		{
			if (checkCollision(*Ball, box))
			{
				if (!box.IsSolid)
				{
					box.Destroyed = GL_TRUE;
				}
			}
		}
	}
}*/

void ActivatePowerUp(PowerUp &powerUp)
{
	if (powerUp.Type == "speed")
	{
		Ball->Velocity *= 1.2;
	}
	else if (powerUp.Type == "sticky")
	{
		Ball->Sticky = GL_TRUE;
		Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
	}
	else if (powerUp.Type == "pass-through")
	{
		Ball->PassThrough = GL_TRUE;
		Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
	}
	else if (powerUp.Type == "pad-size-increase")
	{
		Player->Size.x += 50;
	}
	else if (powerUp.Type == "confuse")
	{
		if (!Effects->Chaos)
		{
			Effects->Confuse = GL_TRUE;
		}
	}
	else if (powerUp.Type == "chaos")
	{
		if (!Effects->Confuse)
		{
			Effects->Chaos = GL_TRUE;
		}
	}
}

void Game::DoCollisions()
{
	for (GameObject& box : this->levels[this->level - 1].Bricks)
	{
		if (!box.Destroyed)
		{
			Collision collision = checkCollision(*Ball, box);
			if (get<0>(collision))
			{
				if (!box.IsSolid)
				{
					SoundEngine->play2D("bleep.mp3", GL_FALSE);
					box.Destroyed = true;
					this->SpawnPowerUps(box);
				}
				else
				{
					SoundEngine->play2D("solid.wav", GL_FALSE);
					ShakeTime = 0.05f;
					Effects->Shake = true;
				}
				Direction dir = get<1>(collision);
				glm::vec2 diff_vector = get<2>(collision);
				if (!(Ball->PassThrough && !box.IsSolid))
				{
					if (dir == LEFT || dir == RIGHT)
					{
						Ball->Velocity.x = -Ball->Velocity.x;
						GLfloat penetration = Ball->Radius - abs(diff_vector.x);
						if (dir == LEFT)
						{
							Ball->Position.x += penetration;
						}
						else
						{
							Ball->Position.x -= penetration;
						}
					}
					else
					{
						Ball->Velocity.y = -Ball->Velocity.y;
						GLfloat penetration = Ball->Radius - abs(diff_vector.y);
						if (dir == UP)
						{
							Ball->Position.y -= penetration;
						}
						else
						{
							Ball->Position.y += penetration;
						}
					}
				}
			}
		}
	}
	Collision result = checkCollision(*Ball, *Player);
	if (!Ball->Stuck && get<0>(result))
	{
		SoundEngine->play2D("bleep.wav", GL_FALSE);
		GLfloat centerBoard = Player->Position.x + Player->Size.x / 2;
		GLfloat distance = (Ball->Position.x + Ball->Radius) - centerBoard;
		GLfloat percentage = distance / (Player->Size.x / 2);

		GLfloat strength = 2.0f;
		glm::vec2 oldVelocity = Ball->Velocity;
		Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
//		Ball->Velocity.y = -Ball->Velocity.y;
		Ball->Velocity.y = -1 * abs(Ball->Velocity.y);
		Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
		Ball->Stuck = Ball->Sticky;
	}

	for (PowerUp &powerUp : this->PowerUps)
	{
		if (!powerUp.Destroyed)
		{
			if (powerUp.Position.y >= this->Height)
			{
				powerUp.Destroyed = GL_TRUE;
			}
			if (checkCollision(*Player, powerUp))
			{
				SoundEngine->play2D("powerup.wav", GL_FALSE);
				ActivatePowerUp(powerUp);
				powerUp.Destroyed = GL_TRUE;
				powerUp.Activated = GL_TRUE;
			}
		}
	}
}

GLboolean ShouldSpawn(GLuint chance)
{
	GLuint random = rand() % chance;
	return random == 0;
}

void Game::SpawnPowerUps(GameObject &block)
{
	if (ShouldSpawn(75))
	{
		this->PowerUps.push_back(PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("powerup_speed")));
	}
	if (ShouldSpawn(75))
	{
		this->PowerUps.push_back(PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, ResourceManager::GetTexture("powerup_sticky")));
	}
	if (ShouldSpawn(75))
	{
		this->PowerUps.push_back(PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, ResourceManager::GetTexture("powerup_passthrough")));
	}
	if (ShouldSpawn(75))
	{
		this->PowerUps.push_back(PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4f), 0.0f, block.Position, ResourceManager::GetTexture("powerup_increase")));
	}
	if (ShouldSpawn(15))
	{
		this->PowerUps.push_back(PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_confuse")));
	}
	if (ShouldSpawn(15))
	{
		this->PowerUps.push_back(PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_chaos")));
	}
}

GLboolean IsOtherPowerUpActive(vector<PowerUp> &powerUps, string type)
{
	for (const PowerUp &p : powerUps)
	{
		if (p.Activated && p.Type == type)
		{
			return GL_TRUE;
		}
	}
	return GL_FALSE;
}

void Game::UpdatePowerUps(GLfloat dt)
{
	for (PowerUp &powerUp : this->PowerUps)
	{
		powerUp.Position += powerUp.Velocity * dt;
		if (powerUp.Activated)
		{
			powerUp.Duration -= dt;
			if (powerUp.Duration <= 0.0f)
			{
				powerUp.Activated = GL_FALSE;
				if (powerUp.Type == "sticky")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "sticky"))
					{
						Ball->Sticky = GL_FALSE;
						Player->Color = glm::vec3(1.0f);
					}
				}
				else if (powerUp.Type == "pass-through")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "pass-through"))
					{
						Ball->PassThrough = GL_FALSE;
						Ball->Color = glm::vec3(1.0f);
					}
				}
				else if (powerUp.Type == "confuse")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "confuse"))
					{
						Effects->Confuse = GL_FALSE;
					}
				}
				else if (powerUp.Type == "chaos")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "chaos"))
					{
						Effects->Chaos = GL_FALSE;
					}
				}
			}
		}
	}
	this->PowerUps.erase(remove_if(this->PowerUps.begin(), this->PowerUps.end(), [](const PowerUp &powerUp){return powerUp.Destroyed && !powerUp.Activated; }), this->PowerUps.end());
}