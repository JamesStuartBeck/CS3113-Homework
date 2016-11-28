/*
Steps for Homework #4 (platformer)

1) Make a map in Tiled  (done by class 11/9/2016)
	-Make sure you can import it
	-Make sure that it can be displayed

	*****What about [tilesets] in the .txt file?*****
	*****What about orientation=orthogonal in the .txt file?*****
	*****You have to make your own placeEntity function, right?*****
	*****After that, how do you know if the map is displayed? If you run it will it make the map? Or do you need like, a draw funcion or something?*****
	*****Draw function for entities still though, right? What about spritesheets and loadImage()?*****

	As you go through the array built, you need to draw those triangles
	Start with a 2D array you build yourself, and then try a tiled map

2) Make a collision function and a push function to go with it (done by class on 11/14/2016)
	-You can't move onto the next step of making an interactive player if you don't have solid surfaces
	-Fixed timestep is mandatory
	-Set certain tiles to solid (ground) and to die on contact (spikes)

3) Make an interactive player (done by class on 11/16/2016)
	-Can move around and not float through solid objects (gravity, friction, acceleration, lerp, velocity to jump, etc)
	-Can interact with one other entity (enemy; will die if he touches it)

Extras
	1) Have a title screen and game states to accompany it
	2) Have more than one enemy, or a power up to let you kill the enemy
	3) Have the player's sprite change when he moves
*/

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "ShaderProgram.h"
#include "Matrix.h"

#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

#define LEVEL_HEIGHT 1 //32
#define LEVEL_WIDTH 10 //128
#define SPRITE_COUNT_X 30
#define SPRITE_COUNT_Y 30
#define TILE_SIZE 23.0f
#define FRICTION_X 2.5f
#define FRICTION_Y 2.5f
#define GRAVITY_X 2.5f
#define GRAVITY_Y 2.5f
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#define SOLID [101, 122, 123, 124, 126, 127, 128, 129, 153, 155, 156]
#define DEADLY [71]

using namespace std;

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

unsigned char levelData[LEVEL_HEIGHT][LEVEL_WIDTH];
unsigned char level1Data[LEVEL_HEIGHT][LEVEL_WIDTH] = { { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } };

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

void buildLevel() {
	memcpy(levelData, level1Data, LEVEL_HEIGHT*LEVEL_WIDTH);
}

void renderMap(ShaderProgram *program, int textureMap) {
	vector<float> vertexData;
	vector<float> texCoordData;
	Matrix modelMatrix;
	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
			if (levelData[y][x] != 0) {
				float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
				float v = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
				float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
				float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
				vertexData.insert(vertexData.end(), {
				
					TILE_SIZE * x, -TILE_SIZE * y,
					TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,

					TILE_SIZE * x, -TILE_SIZE * y,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
				});

				texCoordData.insert(texCoordData.end(), {
				
				u, v,
				u, v + (spriteHeight),
				u + spriteWidth, v + (spriteHeight),

				u, v,
				u+spriteWidth, v+spriteHeight,
				u+spriteWidth, v
				});

				glUseProgram(program->programID);
				glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
				glEnableVertexAttribArray(program->positionAttribute);
				glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
				glEnableVertexAttribArray(program->texCoordAttribute);
				glBindTexture(GL_TEXTURE_2D, textureMap);
				program->setModelMatrix(modelMatrix);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glDisableVertexAttribArray(program->positionAttribute);
				glDisableVertexAttribArray(program->texCoordAttribute);
			}
		}
	}
}

float lerp(float v0, float v1, float t) {
	return (1.0f - t)*v0 + t*v1;
}

bool collision(int *gridX, int gridY) {

}

// convert world position to tile position (different function called within collision)
// see if the tiles that are being solid or not, or deadly or not (collision) (different function called within collision function)
// if so, set velocity to 0 and push out by penetration + a small amount (push in opposite direction of velocity)
// y position check to see if bottom collides, then check top
// x position check to see if left collides, then right

class Entity {
public:
	Entity(std::vector<float> position, std::vector<float> velocity, vector<float> acceleration, vector<float> size, float rotation, int spriteSheetTexture, int index) :
		position(position), velocity(velocity), acceleration(acceleration), size(size), rotation(rotation), spriteSheetTexture(spriteSheetTexture), index(index) {};
	void Draw(ShaderProgram *program) {
		float u = (float)(((int)index) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		float v = (float)(((int)index) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
		float spriteWidth = 1.0f/(float)SPRITE_COUNT_X;
		float spriteHeight = 1.0f/(float)SPRITE_COUNT_Y;
		Matrix modelMatrix;

		float vertices[] = {-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f};

		GLfloat texCoords[] = {
		u, v+spriteHeight,
		u+spriteWidth, v,
		u, v,
		u+spriteWidth, v,
		u, v+spriteHeight,
		u+spriteWidth, v+spriteHeight
		};

		glUseProgram(program->programID);
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program->texCoordAttribute);
		glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);
		modelMatrix.identity();
		modelMatrix.Translate(position[0], position[1], 0.0f);
		program->setModelMatrix(modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	};
	void Update(float elapsed) {
		velocity[1] = lerp(velocity[1], 0.0f, elapsed*FRICTION_Y);
		velocity[1] += GRAVITY_Y * elapsed;
		velocity[1] += acceleration[1] * elapsed;
		position[1] += velocity[1] * elapsed;
		// check for y collision and fix if so
		velocity[0] = lerp(velocity[0], 0.0f, elapsed * FRICTION_X);
		velocity[0] += GRAVITY_X * elapsed;
		velocity[0] += acceleration[0] * elapsed;
		position[0] += velocity[0] * elapsed;
		// check for x collision and fix if so
	}
	std::vector<float> position;
	std::vector<float> velocity;
	std::vector<float> acceleration;
	std::vector<float> size;
	float rotation;
	int spriteSheetTexture;
	int index;
private:
	void collisionY() {
		int gridY = (int)(-position[1]/TILE_SIZE);
		// already converted to grid coordinates
		// check to see if position - height/2 is eithin a tile ( < gridY???) -> if so, there is a collision
		// if that tile is within DEADLY, end game (make entity have a bool called "alive" and check if it is true. If it is false, then "done" is true)
		// if that tile is not within DEADLY but is within solid, then calculate penetration, push back in the opposite direction of velocity until penetration is 0, and thern set velocity to 0
	}
	void collisionX() {
		int gridX = (int)(position[0] / TILE_SIZE);
	}
};

void Update(std::vector<Entity> entities, float elapsed) {
	for (int i = 0; i < entities.size(); ++i) {
		entities[i].Update(elapsed);
	}
}

/*
bool readHeader(istream &stream) {
	string line;
	mapWidth = -1;
	mapHeight = -1;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "width") {
			mapWidth = atoi(value.c_str());
		}
		else if (key == "height") {
			mapHeight = atoi(value.c_str());
		}
	}
	if (mapWidth == -1 || mapHeight == -1) {
		return false;
	}
	else {
		levelData = new unsigned char*[mapHeight];
		for (int i = 0; i < mapHeight; ++i) {
			levelData[i] = new unsigned char[mapWidth];
		}
		return true;
	}
}

bool readLayerData(ifstream &stream) {
	string line;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "data") {
			for (int y = 0; y < mapHeight; y++) {
				getline(stream, line);
				istringstream lineStream(line);
				string tile;
				for (int x = 0; x < mapWidth; x++) {
					getline(lineStream, tile, ',');
					unsigned char val = (unsigned char)atoi(tile.c_str());
					if (val > 0) {
						levelData[y][x] = val - 1;
					}
					else {
						levelData[y][x] = 0;
					}
				}
			}
		}
	}
	return true;
}

bool readEntityData(ifstream &stream) {
	string line;
	string type;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "type") {
			type = value;
		}
		else if (key == "location") {
			istringstream lineStream(value);
			string xPosition, yPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');
			float placeX = atoi(xPosition.c_str())*TILE_SIZE;
			float placeY = atoi(yPosition.c_str())*-TILE_SIZE;
			placeEntity(type, placeX, placeY);
		}
	}
	return true;
}
*/

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 360);

	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Matrix projectionMAtrix;
	Matrix viewMatrix;

	projectionMAtrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	GLuint spriteSheetTexture = LoadTexture("spritesheet_rgba.png");

	Entity player({ 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.5f, 0.5f }, 0.0f, spriteSheetTexture, 79);
	std::vector<Entity> entities;
	entities.push_back(player);

	/*
	ifstream infile(levelFile);
	string line;
	while (getline(infile, line)) {
		if (line == "[header]") {
			if (!readHeader(infile)) {
				return;
			}
		}
		else if (line == "[layer]") {
			readLayerData(infile);
		}
		else if (line == "[Object Layer 1]") {
			readEntityData(infile);
		}
	}
	*/

	float lastFrameTicks = 0.0f;

	glUseProgram(program.programID);

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYUP) {
				if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
					player.velocity[1] = 1.0f;
				}
			}
		}

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		player.acceleration = { 0.0f, 0.0f };

		if (keys[SDL_SCANCODE_LEFT]) {
			player.acceleration[0] = -1.0f;
		}
		else if (keys[SDL_SCANCODE_RIGHT]) {
			player.acceleration[0] = 1.0f;
		}

		buildLevel();
		renderMap(&program, spriteSheetTexture);

		float fixedElapsed = elapsed;
		if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
			fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
		}
		while (fixedElapsed >= FIXED_TIMESTEP) {
			fixedElapsed -= FIXED_TIMESTEP;
			Update(entities, FIXED_TIMESTEP);
		}
		Update(entities, fixedElapsed);

		viewMatrix.identity();
		viewMatrix.Translate(player.position[0], player.position[1], 0.0f);

		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}