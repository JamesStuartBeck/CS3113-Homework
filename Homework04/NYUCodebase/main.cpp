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

3) Make an interactive player (done by class on 11/16/2016)
	-Can move around and not float through solid objects
	-Can interact with one other entity (enemy; will die if he touches it)
	-Have the view matrix move with the player

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

#define LEVEL_HEIGHT 32
#define LEVEL_WIDTH 128
#define SPRITE_COUNT_X
#define SPRITE_COUNT_Y
#define TILE_SIZE

using namespace std;

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

unsigned char levelData[LEVEL_HEIGHT][LEVEL_WIDTH];

void buildLevel() {
	memcpy(levelData, level1Data, LEVEL_HEIGHT*LEVEL_WIDTH);
}

void renderMap() {
	vector<float> vertexData;
	vector<float> texCoordData;
	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
			if (levelData[y][x] != 0) {
				float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float) SPRITE_COUNT_X;

			}
		}
	}
}

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

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}