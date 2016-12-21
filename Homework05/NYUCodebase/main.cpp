/*

make the normal vector for each edge of the figure
	-find the edge (if you have 2 point, (x1. y1) and (x2, y2), the edge is (x2 - x1, y2 - y1))
	-normals are (-(x2 - x1), y2 - y1) and (x2 - x1, -(y2 - y1))
	-normalize by dividing by length ((x2 - x1)^2 + (y2 - y1)^2)
project each shape onto the normal vector
	-find the dot product of each vertex (x position of a corner, y position of a corner)(can be calculated by position + or - height or width / 2)
		and the center with the normalized normal vector calculated earlier
	-dot product = (x1 * x2) + (y1 * y2)
check for a collision
	-distance = | shape1 center - shape2 center | - (((max vertex shape 1 - min vertex shape 1) + (max vertex shape 2 - min vertex shape 2)) / 2)
	- if distance > 0 for any of the normal vectors, then there is not a collision
adjust
	-if there is a collision, move them away from each other along that unit vector by a small amount
	-see if code on slide 52 helps at all

code provided
	-testSAT (test it on each edge, with the vector of points already made)
		-takes two edges and 2 vectors of vectors of points
			-edgeX = x2 - x1
			-edgeY = y2 - y1
			-vector has 2 data members
				-both floats
				-x coord of the point
				-y coord of the point
			-takes a vector of those vectors
		-calculates normals, normalizes, projects, and checks for a collision
	-checkSAT
		-takes 2 vectors of vectors (similar to points in testSAT)
		-calculates edge and checks if there is a collision with points using testSAT


assignment
	-simple seperated axis collision demo using colliding rectrangles or polygons
	-3 objects
	-rotated and scaled

make 3 shapes (untextured polygons)
rotate and scale them
find their corners and feed it into the checkSAT
	-check shape 1 with 2 and 3
	-check shape 2 with shape 3
adjust using the code on slide 52
https://gist.github.com/ivansafrin/3c1deea6b52477d61c21294828dcc2f0
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
#include <algorithm>

using namespace std;

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

class Vector {
public:
	Vector(float x, float y) : x(x), y(y) {};
	float x;
	float y;

	void normalize() {
		float length = sqrtf((x * x) + (y * y));
		x /= length;
		y /= length;
	}
};

class Entity{
public:
	Entity(Vector position, Vector velocity, Vector size, float rotation, float scale) :
		position(position), velocity(velocity), size(size), rotation(rotation), scale(scale) {};

	Vector position;
	Vector velocity;
	Vector size;
	float rotation;
	float scale;

	void Update(float elapsed) {
		position.x += velocity.x * elapsed;
		position.y += velocity.y * elapsed;
		if (position.x > 3.55f) {
			float penetration = fabs(position.x - 3.55f);
			position.x -= penetration;
			velocity.x *= -1;
		}
		else if (position.x < -3.55f) {
			float penetration = fabs(position.x + 3.55f);
			position.x += penetration;
			velocity.x *= -1;
		}
		if (position.y > 2.0f) {
			float penetration = fabs(position.y - 2.0f);
			position.y -= penetration;
			velocity.y *= -1;
		}
		else if (position.y < 2.0f) {
			float penetration = fabs(position.x + 2.0f);
			position.y += penetration;
			velocity.y *= -1;
		}
		rotation += elapsed;
	}

	void Draw(ShaderProgram *program) {
		Matrix modelMatrix;
		float vertices[] = { -0.5f, -0.5f,
			0.5f, 0.5f,
			-0.5f, 0.5f,
			0.5f , 0.5f,
			-0.5f, -0.5f,
			0.5f, -0.5f, };
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);
		modelMatrix.identity();
		modelMatrix.Translate(position.x, position.y, 0.0f);
		modelMatrix.Rotate(rotation);
		modelMatrix.Scale(scale, scale, 1.0f);
		program->setModelMatrix(modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program->positionAttribute);
	};
};

bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector> &points1, const std::vector<Vector> &points2) {
	float normalX = -edgeY;
	float normalY = edgeX;
	float len = sqrtf(normalX*normalX + normalY*normalY);
	normalX /= len;
	normalY /= len;

	std::vector<float> e1Projected;
	std::vector<float> e2Projected;

	for (int i = 0; i < points1.size(); i++) {
		e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
	}
	for (int i = 0; i < points2.size(); i++) {
		e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
	}

	std::sort(e1Projected.begin(), e1Projected.end());
	std::sort(e2Projected.begin(), e2Projected.end());

	float e1Min = e1Projected[0];
	float e1Max = e1Projected[e1Projected.size() - 1];
	float e2Min = e2Projected[0];
	float e2Max = e2Projected[e2Projected.size() - 1];
	float e1Width = fabs(e1Max - e1Min);
	float e2Width = fabs(e2Max - e2Min);
	float e1Center = e1Min + (e1Width / 2.0);
	float e2Center = e2Min + (e2Width / 2.0);
	float dist = fabs(e1Center - e2Center);
	float p = dist - ((e1Width + e2Width) / 2.0);

	if (p < 0) {
		return true;
	}
	return false;
}

bool checkSATCollision(const std::vector<Vector> &e1Points, const std::vector<Vector> &e2Points) {
	for (int i = 0; i < e1Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e1Points.size() - 1) {
			edgeX = e1Points[0].x - e1Points[i].x;
			edgeY = e1Points[0].y - e1Points[i].y;
		}
		else {
			edgeX = e1Points[i + 1].x - e1Points[i].x;
			edgeY = e1Points[i + 1].y - e1Points[i].y;
		}

		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
		if (!result) {
			return false;
		}
	}
	for (int i = 0; i < e2Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e2Points.size() - 1) {
			edgeX = e2Points[0].x - e2Points[i].x;
			edgeY = e2Points[0].y - e2Points[i].y;
		}
		else {
			edgeX = e2Points[i + 1].x - e2Points[i].x;
			edgeY = e2Points[i + 1].y - e2Points[i].y;
		}
		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
		if (!result) {
			return false;
		}
	}
	return true;
}

bool checkCollision(Entity square, Entity rectangle) {

}

SDL_Window* displayWindow;

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

	ShaderProgram program(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Matrix projectionMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);


	Vector vec(1.0f, 1.0f);

	vector<Entity> entities;
	Entity square(Vector(1.0f, 1.0f), Vector(1.0f, 1.0f), Vector(1.0f, 1.0f), 1.0f, 1.0f);
	Entity rectangle(Vector(-2.5f, 0.5f), Vector(-1.0f, 0.5f), Vector(1.0f, 1.0f), 0.0f, 1.0f);
	Entity anotherSquare(Vector(0.0f, 0.0f), Vector(0.25f, -3.0f), Vector(2.0f, 2.0f), 0.0f, 2.0f);

	entities.push_back(square);
	entities.push_back(rectangle);
	entities.push_back(anotherSquare);

	float lastFrameTicks = 0.0f;

	glUseProgram(program.programID);

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		// loops through entities and checks 1 & 2, 1 & 3, and 2 & 3
		int maxChecks = 10;
		while (checkCollision() && maxChecks > 0) {
			Vector responseVector = Vector();
			responseVector.normalize();
			-= responseVector.x * 0.002;
			-= responseVector.y * 0.002;
			+= responseVector.x * 0.002;
			+= responseVector.y * 0.002;
		}

		for (int 1 = 0; i < entities.size(); i++) {
			entities[i].Update(elapsed);
		}

		for (int 1 = 0; i < entities.size(); i++) {
			entities[i].Draw(&program);
		}

		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}