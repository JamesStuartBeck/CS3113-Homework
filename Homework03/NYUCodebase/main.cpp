/*
Aliens randomly jump and I don't know why (probably just a glitch that happens infrequently, but still annoying)
Laser sound effect won't work
Music doesn't play
*/

#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "ShaderProgram.h"
#include "Matrix.h"
#include <SDL_mixer.h>

#include <vector>

using namespace std;

#define MAX_LASERS 30
#define MAX_ALIENS 40

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint LoadTexture(const char *image_path){
	SDL_Surface *surface = IMG_Load(image_path);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SDL_FreeSurface(surface);
	return textureID;
}

class SheetSprite {
public:
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size, ShaderProgram *program) : 
		textureID(textureID), u(u), v(v), width(width), height(height), size(size) {};
	void Draw(ShaderProgram *program, float xPos, float yPos) {
		glBindTexture(GL_TEXTURE_2D, textureID);
		Matrix modelMatrix;
		GLfloat texCoords[] = { u, v + height, u + width, v, u, v, u + width, v, u, v + height, u + width, v + height };
		float aspect = width / height;
		float vertices[] = { -0.5f * size * aspect, -0.5f * size,
			0.5f * size * aspect, 0.5f * size,
			-0.5f * size * aspect, 0.5f * size,
			0.5f * size * aspect, 0.5f * size,
			-0.5f * size * aspect, -0.5f * size,
			0.5f * size * aspect, -0.5f * size, };
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program->texCoordAttribute);
		modelMatrix.identity();
		modelMatrix.Translate(xPos, yPos, 0.0f);
		program->setModelMatrix(modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	};
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
	float size;
};

class Entity {
public:
	Entity(std::vector<float> position, std::vector<float> velocity, vector<float> size, float rotation, SheetSprite sprite) :
		position(position), velocity(velocity), size(size), rotation(rotation), sprite(sprite) {};
	void Draw(ShaderProgram *program) {
		sprite.Draw(program, position[0], position[1]);
	};
	std::vector<float> position;
	std::vector<float> velocity;
	std::vector<float> size;
	float rotation;
	SheetSprite sprite;
};

void drawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing, float xPos, float yPos) {
	float texture_size = 1.0f / 16.0f;
	vector<float> vertexData;
	vector<float> texCoordData;
	Matrix modelMatrix;
	for (int i = 0; i < text.size(); i++) {
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size});
		texCoordData.insert(texCoordData.end(), {
		texture_x, texture_y,
		texture_x, texture_y + texture_size,
		texture_x + texture_size, texture_y,
		texture_x + texture_size, texture_y + texture_size,
		texture_x + texture_size, texture_y,
		texture_x, texture_y + texture_size});
	}
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	modelMatrix.identity();
	modelMatrix.Translate(xPos, yPos, 0.0f);
	program->setModelMatrix(modelMatrix);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

enum GameState { STATE_MAIN_MENU = 0, INSTRUCTION_STATE = 1, STATE_GAME_LEVEL = 2, STATE_GAME_OVER = 3 , STATE_VICTORY = 4};
int state = 0;

vector<Entity> lasers;
int lasersIterator = 0;
int deadAliens = 0;

void shootLaser(float positionX, float positionY) {
	lasers[lasersIterator].velocity[1] = 5.0f;
	lasers[lasersIterator].position = { positionX, positionY };
	if (lasersIterator < MAX_LASERS) {
		lasersIterator++;
	}
	else {
		lasersIterator = 0;
	}
}

bool laserCollision(float laserX, float laserY, float shipX, float shipY, float width, float height) {
	if (((laserX < shipX + width/2) && (laserX > shipX - width/2)) && ((laserY > shipY - height/2) && (laserY < shipY + height/2))) {
		return true;
	}
	else {
		return false;
	}
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 448, 512, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 448, 512);

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Matrix projectionMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-2.24, 2.24, -2.56f, 2.56f, -1.0f, 1.0f);

	// Loads the spritesheet for the font and the ships and lasers
	GLuint spriteSheetTexture = LoadTexture("sheet.png");
	GLuint fontTexture = LoadTexture("font1.png");

	// Gets the sprites for the player, alien, and laser
	SheetSprite playerShip(spriteSheetTexture, 224.0f / 1024.0f, 832.0f / 1024.0f, 99.0f / 1024.0f, 75.0f / 1024.0f, 0.2f, &program);
	SheetSprite enemyShip(spriteSheetTexture, 434.0f / 1024.0f, 234.0f / 1024.0f, 91.0f / 1024.0f, 91.0f / 1024.0f, 0.2f, &program);
	SheetSprite playerLaser(spriteSheetTexture, 858.0f / 1024.0f, 230.0f / 1024.0f, 9.0f / 1024.0f, 54.0f / 1024.0f, 0.2f, &program);

	// Creates the player
	Entity player({ 0.0f, -1.75f }, { 0.0f, 0.0f }, { 0.2f, 0.2f }, 0.0f, playerShip);

	// Creates the vector of aliens and fills it with aliens
	std::vector<Entity> aliens;
	for (float i = -1.0f; i <= 1.25; i += 0.25) {
		for (float j = 2.25; j > 1.25; j -= 0.25) {
			Entity enemy({ i, j }, { 0.25f, 0.0f }, { 0.2f, 0.2f }, 0.0f, enemyShip);
			aliens.push_back(enemy);
		}
	}

	// Creates the array of lasers, and keeps them "off screen"
	for (int i = 0; i <= MAX_LASERS; i++) {
		Entity newLaser({ 5.0f, 5.0f }, { 0.0f, 0.0f }, { 0.2f, 0.2f }, 0.0f, playerLaser);
		lasers.push_back(newLaser);
	}

	Mix_Chunk *laserSound;
	laserSound = Mix_LoadWAV("laser.wav");

	Mix_Chunk *explosion;
	explosion = Mix_LoadWAV("explosion.wav");

	Mix_Music *music;
//	music = Mix_LoadMUS("music.mp3");

	float lastFrameTicks = 0.0f;

	glUseProgram(program.programID);

	//Mix_PlayMusic(music, -1);

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYUP) {
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					if (state == 0 || state == 1) {
						state++;
					}
					else if (state == 2) {
						// Pressing the space bar in the game makes the player shoot
						// Used SDL_KEYUP to prevent a player from holding down the space bar and killing all the aliens easily
						shootLaser(player.position[0], player.position[1]);
						Mix_PlayChannel(-1, laserSound, 0);
					}
				}
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);

		if (state == 0) {
			// Main Menu
			drawText(&program, fontTexture, "Space Invaders", 0.3f, 0.01f, -2.0f, 2.0f);
			drawText(&program, fontTexture, "By James Beck", 0.25f, 0.01f, -1.6f, 1.5f);
			drawText(&program, fontTexture, "Press the space", 0.25f, 0.01f, -1.85f, -1.0f);
			drawText(&program, fontTexture, "bar to start", 0.25f, 0.01f, -1.5f, -1.25f);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
		}
		else if (state == 1) {
			// Instructions
			drawText(&program, fontTexture, "Instructions", 0.3f, 0.01f, -1.7f, 2.0f);
			drawText(&program, fontTexture, "Arrows to move", 0.25f, 0.01f, -1.7f, 0.0f);
			drawText(&program, fontTexture, "Space to shoot", 0.25f, 0.01f, -1.7f, -0.5f);
			drawText(&program, fontTexture, "Don't let the aliens", 0.20f, 0.01f, -1.95f, -1.0f);
			drawText(&program, fontTexture, "pass the red line", 0.20f, 0.01f, -1.7f, -1.25f);

			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);

			// Starts monitoring ticks so that if the player stays on this screen, the aliens won't "jump" to their positions when the player advances the game state
			float ticks = (float)SDL_GetTicks() / 1000.0f;
			float elapsed = ticks - lastFrameTicks;
			lastFrameTicks = ticks;
		}
		else if (state == 2) {
			float ticks = (float)SDL_GetTicks() / 1000.0f;
			float elapsed = ticks - lastFrameTicks;
			lastFrameTicks = ticks;

			// Checks if all the aliens are dead, and advances to the "victory" stage if they are all dead
			if (deadAliens == MAX_ALIENS) {
				state = 4;
			}

			const Uint8 *keys = SDL_GetKeyboardState(NULL);

			player.velocity[0] = 0.0f;

			if (keys[SDL_SCANCODE_LEFT]) {
				player.velocity[0] = -2.0f;
			}
			else if (keys[SDL_SCANCODE_RIGHT]) {
				player.velocity[0] = 2.0f;
			}

			player.position[0] += player.velocity[0] * elapsed;

			// Makes the red line on the screen
			float lineVertices[] = {-2.5f, -1.4f, 2.5f, -1.4f, -2.5, -1.5f, 2.5f, -1.4f, 2.5f, -1.5f, -2.5, -1.5f};
			// Stretched a red power up from the sprite sheet to fit the screen
			float texCoordVertices[] = { 491.0f/1024.0f, 187.0f/1024.0f, 525.0f/1024.0f, 187.0f/1024.0f, 491.0f/1024.0f, 200.0f/1024.0f, 525.0f/1024.0f, 187.0f/1024.0f, 525.0f/1024.0f, 200.0f/1024.0f, 491.0f/1024.0f, 200.0f/1024.0f };
			Matrix lineModelMatrix;
			glUseProgram(program.programID);
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, lineVertices);
			glEnableVertexAttribArray(program.positionAttribute);
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordVertices);
			glEnableVertexAttribArray(program.texCoordAttribute);
			program.setModelMatrix(lineModelMatrix);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(program.positionAttribute);
			glDisableVertexAttribArray(program.texCoordAttribute);

			// Checks for collision with sides of the screen
			if (player.position[0] <= -2.24 + (player.size[0] / 2)) {
				player.velocity[0] = 0.0f;
				player.position[0] = -2.23 + (player.size[0] / 2);
			}

			if (player.position[0] >= 2.24 - (player.size[0] / 2)) {
				player.velocity[0] = 0.0f;
				player.position[0] = 2.23 - (player.size[0] / 2);
			}

			player.Draw(&program);

			// Checks for laser collision
			// Loops through vector of lasers and compares position to alien position
			for (int i = 0; i < lasers.size(); ++i) {
				lasers[i].position[1] += lasers[i].velocity[1] * elapsed;
				for (int j = 0; j < aliens.size(); j++) {
					if (laserCollision(lasers[i].position[0], lasers[i].position[1], aliens[j].position[0], aliens[j].position[1], aliens[j].size[0], aliens[j].size[1])) {
						// If there is a collision, moves the laser and the alien off of the screen
						lasers[i].velocity[1] = 0.0f;
						lasers[i].position = { 5.0f, 5.0f };
						aliens[j].velocity[0] = 0.0f;
						aliens[j].position = { 0.0f, 20.0f };
						deadAliens++;
						Mix_PlayChannel(-1, explosion, 0);
						// Speeds up alive aliens when an alien dies
						for (int k = 0; k < aliens.size(); k++) {
							// But only applies the update to aliens that are on the screen
							if (aliens[k].position[1] < 5.0f) {
								if (aliens[k].velocity[0] < 0.0f) {
									aliens[k].velocity[0] -= 0.1f;
								}
								else {
									aliens[k].velocity[0] += 0.1f;
								}
							}
						}
					}
				}

				// Lasers that miss are sent off screen
				if (lasers[i].position[1] > 2.56) {
					lasers[i].position = { 5.0f, 5.0f };
				}
				lasers[i].Draw(&program);
			}

			// When aliens hit the red line, game is over
			for (int i = 0; i < aliens.size(); ++i) {
				if (aliens[i].position[1] <= -1.5f) {
					state++;
					break;
				}
				// Reverses direction of alien and moves them forward one unit when one alien hits the wall
				if (aliens[i].position[0] >= 2.24f - (aliens[i].size[0] / 2) || aliens[i].position[0] <= -2.24f + (aliens[i].size[0])) {
					for (int j = 0; j < aliens.size(); j++) {
						aliens[j].velocity[0] *= -1;
						aliens[j].position[1] -= 0.25;
					}
					break;
				}
			}

			for (int i = 0; i < aliens.size(); ++i) {
				aliens[i].position[0] += aliens[i].velocity[0] * elapsed;
				aliens[i].Draw(&program);
			}

			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);

		}
		else if (state == 3 ) {
			// Game over state
			drawText(&program, fontTexture, "Game Over", 0.4f, 0.01f, -1.6f, 0.5f);
			drawText(&program, fontTexture, "You defeated", 0.25f, 0.01f, -1.4f, -0.5f);
			// Tells the player how many aliens they killed
			drawText(&program, fontTexture, to_string(deadAliens) + " Aliens", 0.25f, 0.01f, -1.0f, -0.75f);
		}
		else {
			// Victory game state
			drawText(&program, fontTexture, "You Win!", 0.4f, 0.01f, -1.4f, 0.5f);
		}

		SDL_GL_SwapWindow(displayWindow);
	}

	Mix_FreeChunk(laserSound);
	Mix_FreeChunk(explosion);
//	Mix_FreeMusic(music);
	SDL_Quit();
	return 0;
}