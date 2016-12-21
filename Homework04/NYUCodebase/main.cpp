/*
Platformer:
	-Collision function is sorta weird. Collisions on the right side of the entity handle differently than collisions on the left side of the entity
	-Admittingly, the collision between the player and the enemy is sorta janky
	-Repurposing the entity class into the enemy class is also pretty janky
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
#define SPRITE_COUNT_X 30
#define SPRITE_COUNT_Y 30
#define TILE_SIZE 0.25f
#define FRICTION_X 4.0f
#define FRICTION_Y 0.0f
#define GRAVITY_X 0.0f
#define GRAVITY_Y -3.0f
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

using namespace std;

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

unsigned int levelData[LEVEL_HEIGHT][LEVEL_WIDTH] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 313, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 313, 0, 0 },
{ 123, 123, 156, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 126, 125, 125 },
{ 153, 153, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 155, 155 },
{ 153, 153, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 155, 155 },
{ 153, 153, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 155, 155 },
{ 153, 153, 0, 0, 0, 0, 0, 631, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 155, 155 },
{ 153, 153, 0, 0, 0, 0, 669, 670, 671, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 155, 155 },
{ 153, 153, 0, 0, 0, 669, 670, 670, 670, 671, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 631, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 155, 155 },
{ 153, 153, 0, 0, 0, 0, 0, 634, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 669, 670, 671, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 155, 155 },
{ 153, 153, 0, 0, 18, 0, 0, 632, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 669, 670, 670, 670, 671, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 155, 155 },
{ 153, 153, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 156, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 669, 670, 670, 670, 670, 670, 671, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 155, 155 },
{ 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 632, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 155, 155 },
{ 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 101, 0, 0, 0, 101, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 631, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 124, 124, 124, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 155, 155 },
{ 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 123, 123, 123, 123, 123, 123, 156, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 123, 123, 123, 123, 123, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 669, 670, 671, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 127, 125, 153, 153, 153, 153, 153, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 155, 155 },
{ 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 153, 153, 153, 153, 153, 153, 153, 124, 128, 644, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 669, 670, 670, 670, 671, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 127, 125, 153, 153, 153, 153, 153, 153, 153, 153, 128, 0, 0, 18, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 18, 0, 155, 155 },
{ 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 18, 0, 18, 0, 0, 0, 18, 0, 0, 127, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 123, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 632, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 129, 129, 129, 0, 0, 126, 122, 122, 122, 122, 122, 122, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 124, 124, 124, 156, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 126, 123, 123, 123, 123, 123, 155, 155 },
{ 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 123, 123, 123, 123, 123, 123, 123, 123, 123, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 123, 123, 123, 156, 0, 0, 0, 126, 123, 123, 123, 123, 123, 123, 156, 0, 0, 0, 126, 122, 122, 122, 156, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 129, 129, 129, 0, 0, 0, 0, 0, 0, 0, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 0, 0, 129, 129, 0, 0, 0, 18, 0, 129, 129, 0, 0, 0, 155, 155, 155, 155, 155, 155, 155 },
{ 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 0, 0, 0, 0, 0, 155, 155, 155, 155, 155, 155, 0, 0, 0, 0, 0, 153, 153, 153, 0, 0, 0, 129, 129, 129, 0, 0, 0, 0, 0, 129, 129, 129, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 0, 0, 0, 0, 0, 129, 129, 129, 0, 0, 0, 0, 0, 0, 155, 155, 155, 155, 155, 155, 155 },
{ 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 0, 0, 0, 0, 0, 155, 155, 155, 155, 155, 155, 0, 0, 0, 0, 0, 153, 153, 153, 0, 0, 0, 0, 0, 0, 0, 129, 129, 129, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 155, 155, 155, 155, 155, 155, 155 },
{ 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 71, 71, 71, 71, 71, 155, 155, 155, 155, 155, 155, 71, 71, 71, 71, 71, 153, 153, 153, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 153, 153, 153, 153, 153, 153, 153, 153, 155, 155, 153, 153, 153, 153, 155, 155, 153, 153, 153, 153, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 153, 155, 155, 155, 155, 155, 155 },
{ 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 153, 155, 153, 153, 153, 153, 153, 153, 153 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

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

void renderMap(ShaderProgram *program, int textureMap) {
	vector<float> vertexData;
	vector<float> texCoordData;
	Matrix modelMatrix;
	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
			if (levelData[y][x] != 0) {
				float u = (float)(((int)levelData[y][x] -1) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
				float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
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
			}
		}
	}

	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, textureMap);
	program->setModelMatrix(modelMatrix);
	glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 2);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

float lerp(float v0, float v1, float t) {
	return (1.0f - t)*v0 + t*v1;
}

class Entity {
public:
	Entity(std::vector<float> position, std::vector<float> velocity, vector<float> acceleration, vector<float> size, float rotation, int spriteSheetTexture, int index, bool alive = TRUE) :
		position(position), velocity(velocity), acceleration(acceleration), size(size), rotation(rotation), spriteSheetTexture(spriteSheetTexture), index(index), alive(alive) {};
	void Draw(ShaderProgram *program) {
		float u = (float)(((int)index) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		float v = (float)(((int)index) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
		float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
		float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
		Matrix modelMatrix;

		float vertices[] = { -0.25f, -0.25f, 0.25f, 0.25f, -0.25f, 0.25f, 0.25f, 0.25f, -0.25f, -0.25f, 0.25f, -0.25f };

		GLfloat texCoords[] = {
			u, v + spriteHeight,
			u + spriteWidth, v,
			u, v,
			u + spriteWidth, v,
			u, v + spriteHeight,
			u + spriteWidth, v + spriteHeight
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
		collisionY();
		velocity[0] = lerp(velocity[0], 0.0f, elapsed * FRICTION_X);
		velocity[0] += GRAVITY_X * elapsed;
		velocity[0] += acceleration[0] * elapsed;
		position[0] += velocity[0] * elapsed;
		collisionX();
	}
	std::vector<float> position;
	std::vector<float> velocity;
	std::vector<float> acceleration;
	std::vector<float> size;
	float rotation;
	int spriteSheetTexture;
	int index;
	bool alive;
private:
	int worldToTileX(float positionX) {
		int worldX = (int)(positionX / TILE_SIZE);
		if (worldX < 0 || worldX > LEVEL_WIDTH) {
			return 0;
		}
		else {
			return worldX;
		}
	}
	int worldToTileY(float positionY) {
		int worldY = (int)(-positionY / TILE_SIZE);
		if (worldY < 0 || worldY > LEVEL_HEIGHT) {
			return 0;
		}
		else {
			return worldY;
		}
	}
	void collisionY() {
		int bottomLeftBlock = levelData[worldToTileY(position[1] - size[1]/2)][worldToTileX(position[0] - size[0]/2)];
		int bottomRightBlock = levelData[worldToTileY(position[1] - size[1]/2)][worldToTileX(position[0] + size[0]/2)];
		if (bottomLeftBlock == 71 || bottomRightBlock == 71) {
			alive = 0;
		}
		else {
			if (bottomLeftBlock == 101 || bottomLeftBlock == 122 || bottomLeftBlock == 123 || bottomLeftBlock == 124 || bottomLeftBlock == 126 || bottomLeftBlock == 127 ||
				bottomLeftBlock == 128 || bottomLeftBlock == 129 || bottomLeftBlock == 153 || bottomLeftBlock == 155 || bottomLeftBlock == 156) {
				float penetration = fabs((-TILE_SIZE * (worldToTileY(position[1]) + 1)) - (position[1] - size[1] / 2));
				position[1] += (penetration + 0.001);
				velocity[1] = 0.0f;
			}
			else if (bottomRightBlock == 101 || bottomRightBlock == 122 || bottomRightBlock == 123 || bottomRightBlock == 124 || bottomRightBlock == 126 || bottomRightBlock == 127 ||
				bottomRightBlock == 128 || bottomRightBlock == 129 || bottomRightBlock == 153 || bottomRightBlock == 155 || bottomRightBlock == 156) {
				float penetration = fabs((-TILE_SIZE * (worldToTileY(position[1]) + 1)) - (position[1] - size[1] / 2));
				position[1] += (penetration + 0.001);
				velocity[1] = 0.0f;
			}
		}
		int topLeftBlock = levelData[worldToTileY(position[1] + size[1] / 2)][worldToTileX(position[0] - size[0] / 2)];
		int topRightBlock = levelData[worldToTileY(position[1] + size[1] / 2)][worldToTileX(position[0] + size[0] / 2)];
		if (topLeftBlock == 71 || topRightBlock == 71) {
			alive = 0;
		}
		else {
			if (topLeftBlock == 101 || topLeftBlock == 122 || topLeftBlock == 123 || topLeftBlock == 124 || topLeftBlock == 126 || topLeftBlock == 127 ||
				topLeftBlock == 128 || topLeftBlock == 129 || topLeftBlock == 153 || topLeftBlock == 155 || topLeftBlock == 156) {
				float penetration = fabs((position[1] + size[1] / 2) - ((-TILE_SIZE * (worldToTileY(position[1]) - 1) - TILE_SIZE)));
				position[1] -= (penetration + 0.001);
				velocity[1] = 0.0f;
			}
			else if (topRightBlock == 101 || topRightBlock == 122 || topRightBlock == 123 || topRightBlock == 124 || topRightBlock == 126 || topRightBlock == 127 ||
				topRightBlock == 128 || topRightBlock == 129 || topRightBlock == 153 || topRightBlock == 155 || topRightBlock == 156) {
				float penetration = fabs((position[1] + size[1] / 2) - ((-TILE_SIZE * (worldToTileY(position[1]) - 1) - TILE_SIZE)));
				position[1] -= (penetration + 0.001);
				velocity[1] = 0.0f;
			}
		}
	}
	void collisionX() {
		int leftTopBlock = levelData[worldToTileY(position[1] + size[1] / 2)][worldToTileX(position[0] - size[0] / 2)];
		int leftBottomBlock = levelData[worldToTileY(position[1] - size[1] / 2)][worldToTileX(position[0] - size[0] / 2)];
		if (leftTopBlock == 71 || leftBottomBlock == 71) {
			alive = 0;
		}
		else {
			if (leftTopBlock == 101 || leftTopBlock == 122 || leftTopBlock == 123 || leftTopBlock == 124 || leftTopBlock == 126 || leftTopBlock == 127 ||
				leftTopBlock == 128 || leftTopBlock == 129 || leftTopBlock == 153 || leftTopBlock == 155 || leftTopBlock == 156) {
				float penetration = fabs(((TILE_SIZE * (worldToTileX(position[0]) - 1)) + TILE_SIZE) - (position[0] - size[0] / 2));
				position[0] += (penetration + 0.001);
				velocity[0] = 0.0f;
			}
			else if (leftBottomBlock == 101 || leftBottomBlock == 122 || leftBottomBlock == 123 || leftBottomBlock == 124 || leftBottomBlock == 126 || leftBottomBlock == 127 ||
				leftBottomBlock == 128 || leftBottomBlock == 129 || leftBottomBlock == 153 || leftBottomBlock == 155 || leftBottomBlock == 156) {
				float penetration = fabs(((TILE_SIZE * (worldToTileX(position[0]) - 1)) + TILE_SIZE) - (position[0] - size[0] / 2));
				position[0] += (penetration + 0.001);
				velocity[0] = 0.0f;
			}
		}
		int rightTopBlock = levelData[worldToTileY(position[0] + size[0] / 2)][worldToTileX(position[1] + size[1] / 2)];
		int rightBottomBlock = levelData[worldToTileY(position[0] - size[0] / 2)][worldToTileX(position[1] + size[1] / 2)];
		if (rightTopBlock == 71 || rightBottomBlock == 71) {
			alive = 0;
		}
		else {
			if (rightTopBlock == 101 || rightTopBlock == 122 || rightTopBlock == 123 || rightTopBlock == 124 || rightTopBlock == 126 || rightTopBlock == 127 ||
				rightTopBlock == 128 || rightTopBlock == 129 || rightTopBlock == 153 || rightTopBlock == 155 || rightTopBlock == 156) {
				float penetration = fabs((position[0] + size[0] / 2) - (TILE_SIZE * (worldToTileX(position[0]) - 1)));
				position[0] -= (penetration + 0.001);
				velocity[0] = 0.0f;
			}
			else if (rightBottomBlock == 101 || rightBottomBlock == 122 || rightBottomBlock == 123 || rightBottomBlock == 124 || rightBottomBlock == 126 || rightBottomBlock == 127 ||
				rightBottomBlock == 128 || rightBottomBlock == 129 || rightBottomBlock == 153 || rightBottomBlock == 155 || rightBottomBlock == 156) {
				float penetration = fabs((position[0] + size[0] / 2) - (TILE_SIZE * (worldToTileX(position[0]) - 1)));
				position[0] -= (penetration + 0.001);
				velocity[0] = 0.0f;
			}
		}
	}
};

class Enemy {
public:
	Enemy(std::vector<float> position, std::vector<float> velocity, vector<float> acceleration, vector<float> size, float rotation, int spriteSheetTexture, int index, bool alive = TRUE) :
		position(position), velocity(velocity), acceleration(acceleration), size(size), rotation(rotation), spriteSheetTexture(spriteSheetTexture), index(index), alive(alive) {};
	void Draw(ShaderProgram *program) {
		float u = (float)(((int)index) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		float v = (float)(((int)index) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
		float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
		float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
		Matrix modelMatrix;

		float vertices[] = { -0.25f, -0.25f, 0.25f, 0.25f, -0.25f, 0.25f, 0.25f, 0.25f, -0.25f, -0.25f, 0.25f, -0.25f };

		GLfloat texCoords[] = {
			u, v + spriteHeight,
			u + spriteWidth, v,
			u, v,
			u + spriteWidth, v,
			u, v + spriteHeight,
			u + spriteWidth, v + spriteHeight
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
		collisionY();
		velocity[0] = lerp(velocity[0], 0.0f, elapsed * FRICTION_X);
		velocity[0] += GRAVITY_X * elapsed;
		velocity[0] += acceleration[0] * elapsed;
		position[0] += velocity[0] * elapsed;
		collisionX();
	}
	std::vector<float> position;
	std::vector<float> velocity;
	std::vector<float> acceleration;
	std::vector<float> size;
	float rotation;
	int spriteSheetTexture;
	int index;
	bool alive;
private:
	int worldToTileX(float positionX) {
		int worldX = (int)(positionX / TILE_SIZE);
		if (worldX < 0 || worldX > LEVEL_WIDTH) {
			return 0;
		}
		else {
			return worldX;
		}
	}
	int worldToTileY(float positionY) {
		int worldY = (int)(-positionY / TILE_SIZE);
		if (worldY < 0 || worldY > LEVEL_HEIGHT) {
			return 0;
		}
		else {
			return worldY;
		}
	}
	void collisionY() {
		int bottomLeftBlock = levelData[worldToTileY(position[1] - size[1] / 2)][worldToTileX(position[0] - size[0] / 2)];
		int bottomRightBlock = levelData[worldToTileY(position[1] - size[1] / 2)][worldToTileX(position[0] + size[0] / 2)];
		if (bottomLeftBlock == 71 || bottomRightBlock == 71) {
			alive = 0;
		}
		else {
			if (bottomLeftBlock == 101 || bottomLeftBlock == 122 || bottomLeftBlock == 123 || bottomLeftBlock == 124 || bottomLeftBlock == 126 || bottomLeftBlock == 127 ||
				bottomLeftBlock == 128 || bottomLeftBlock == 129 || bottomLeftBlock == 153 || bottomLeftBlock == 155 || bottomLeftBlock == 156) {
				float penetration = fabs((-TILE_SIZE * (worldToTileY(position[1]) + 1)) - (position[1] - size[1] / 2));
				position[1] += (penetration + 0.001);
				velocity[1] = 0.0f;
			}
			else if (bottomRightBlock == 101 || bottomRightBlock == 122 || bottomRightBlock == 123 || bottomRightBlock == 124 || bottomRightBlock == 126 || bottomRightBlock == 127 ||
				bottomRightBlock == 128 || bottomRightBlock == 129 || bottomRightBlock == 153 || bottomRightBlock == 155 || bottomRightBlock == 156) {
				float penetration = fabs((-TILE_SIZE * (worldToTileY(position[1]) + 1)) - (position[1] - size[1] / 2));
				position[1] += (penetration + 0.001);
				velocity[1] = 0.0f;
			}
		}
		int topLeftBlock = levelData[worldToTileY(position[1] + size[1] / 2)][worldToTileX(position[0] - size[0] / 2)];
		int topRightBlock = levelData[worldToTileY(position[1] + size[1] / 2)][worldToTileX(position[0] + size[0] / 2)];
		if (topLeftBlock == 71 || topRightBlock == 71) {
			alive = 0;
		}
		else {
			if (topLeftBlock == 101 || topLeftBlock == 122 || topLeftBlock == 123 || topLeftBlock == 124 || topLeftBlock == 126 || topLeftBlock == 127 ||
				topLeftBlock == 128 || topLeftBlock == 129 || topLeftBlock == 153 || topLeftBlock == 155 || topLeftBlock == 156) {
				float penetration = fabs((position[1] + size[1] / 2) - ((-TILE_SIZE * (worldToTileY(position[1]) - 1) - TILE_SIZE)));
				position[1] -= (penetration + 0.001);
				velocity[1] = 0.0f;
			}
			else if (topRightBlock == 101 || topRightBlock == 122 || topRightBlock == 123 || topRightBlock == 124 || topRightBlock == 126 || topRightBlock == 127 ||
				topRightBlock == 128 || topRightBlock == 129 || topRightBlock == 153 || topRightBlock == 155 || topRightBlock == 156) {
				float penetration = fabs((position[1] + size[1] / 2) - ((-TILE_SIZE * (worldToTileY(position[1]) - 1) - TILE_SIZE)));
				position[1] -= (penetration + 0.001);
				velocity[1] = 0.0f;
			}
		}
	}
	void collisionX() {
		int leftTopBlock = levelData[worldToTileY(position[1] + size[1] / 2)][worldToTileX(position[0] - size[0] / 2)];
		int leftBottomBlock = levelData[worldToTileY(position[1] - size[1] / 2)][worldToTileX(position[0] - size[0] / 2)];
		if (leftTopBlock == 71 || leftBottomBlock == 71) {
			alive = 0;
		}
		else {
			if (leftTopBlock == 101 || leftTopBlock == 122 || leftTopBlock == 123 || leftTopBlock == 124 || leftTopBlock == 126 || leftTopBlock == 127 ||
				leftTopBlock == 128 || leftTopBlock == 129 || leftTopBlock == 153 || leftTopBlock == 155 || leftTopBlock == 156) {
				float penetration = fabs(((TILE_SIZE * (worldToTileX(position[0]) - 1)) + TILE_SIZE) - (position[0] - size[0] / 2));
				position[0] += (penetration + 0.001);
				velocity[0] = 0.0f;
				acceleration[0] *= -1;
			}
			else if (leftBottomBlock == 101 || leftBottomBlock == 122 || leftBottomBlock == 123 || leftBottomBlock == 124 || leftBottomBlock == 126 || leftBottomBlock == 127 ||
				leftBottomBlock == 128 || leftBottomBlock == 129 || leftBottomBlock == 153 || leftBottomBlock == 155 || leftBottomBlock == 156) {
				float penetration = fabs(((TILE_SIZE * (worldToTileX(position[0]) - 1)) + TILE_SIZE) - (position[0] - size[0] / 2));
				position[0] += (penetration + 0.001);
				velocity[0] = 0.0f;
				acceleration[0] *= -1;
			}
		}
		int rightTopBlock = levelData[worldToTileY(position[0] + size[0] / 2)][worldToTileX(position[1] + size[1] / 2)];
		int rightBottomBlock = levelData[worldToTileY(position[0] - size[0] / 2)][worldToTileX(position[1] + size[1] / 2)];
		if (rightTopBlock == 71 || rightBottomBlock == 71) {
			alive = 0;
		}
		else {
			if (rightTopBlock == 101 || rightTopBlock == 122 || rightTopBlock == 123 || rightTopBlock == 124 || rightTopBlock == 126 || rightTopBlock == 127 ||
				rightTopBlock == 128 || rightTopBlock == 129 || rightTopBlock == 153 || rightTopBlock == 155 || rightTopBlock == 156) {
				float penetration = fabs((position[0] + size[0] / 2) - (TILE_SIZE * (worldToTileX(position[0]) - 1)));
				position[0] -= (penetration + 0.001);
				velocity[0] = 0.0f;
				acceleration[0] *= -1;
			}
			else if (rightBottomBlock == 101 || rightBottomBlock == 122 || rightBottomBlock == 123 || rightBottomBlock == 124 || rightBottomBlock == 126 || rightBottomBlock == 127 ||
				rightBottomBlock == 128 || rightBottomBlock == 129 || rightBottomBlock == 153 || rightBottomBlock == 155 || rightBottomBlock == 156) {
				float penetration = fabs((position[0] + size[0] / 2) - (TILE_SIZE * (worldToTileX(position[0]) - 1)));
				position[0] -= (penetration + 0.001);
				velocity[0] = 0.0f;
				acceleration[0] *= -1;
			}
		}
	}
};

bool collideWithEnemy(Entity player, Enemy enemy) {
	if (((player.position[0] < enemy.position[0] + enemy.size[0] / 2) && (player.position[0] > enemy.position[0] - enemy.size[0] / 2)) && ((player.position[1] > enemy.position[1] - enemy.size[1] / 2) && (player.position[1] < enemy.position[1] + enemy.size[1] / 2))) {
		return true;
	}
	else {
		return false;
	}
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

	Matrix projectionMAtrix;
	Matrix viewMatrix;

	projectionMAtrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	GLuint spriteSheetTexture = LoadTexture("spritesheet_rgba.png");

	Entity player({ 1.0f, 5.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.5f, 0.5f }, 0.0f, spriteSheetTexture, 79, 1);
	Enemy enemy({ 3.0f, 5.0f }, { 0.0f, 0.0f }, { -1.0f, 0.0f }, { 0.5f, 0.5f }, 0.0f, spriteSheetTexture, 470, 1);

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
					player.velocity[1] = 2.0f;
				}
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		player.acceleration[0] = 0.0f;

		if (keys[SDL_SCANCODE_LEFT]) {
			player.acceleration[0] = -10.0f;
		}
		else if (keys[SDL_SCANCODE_RIGHT]) {
			player.acceleration[0] = 10.0f;
		}

		renderMap(&program, spriteSheetTexture);

		float fixedElapsed = elapsed;
		if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
			fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
		}
		while (fixedElapsed >= FIXED_TIMESTEP) {
			fixedElapsed -= FIXED_TIMESTEP;
			player.Update(FIXED_TIMESTEP);
			enemy.Update(FIXED_TIMESTEP);
			if (collideWithEnemy(player, enemy)) {
				done = true;
			}
		}

		player.Update(elapsed);
		enemy.Update(elapsed);
		if (collideWithEnemy(player, enemy)) {
			done = true;
		}
		player.Draw(&program);
		enemy.Draw(&program);

		program.setProjectionMatrix(projectionMAtrix);
		viewMatrix.identity();
		viewMatrix.Translate(-player.position[0], -player.position[1], 0.0f);
		program.setViewMatrix(viewMatrix);

		if (!player.alive) {
			done = true;
		}

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}