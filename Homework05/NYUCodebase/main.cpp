/*
I could not get the collision function to work
For some odd reason, my program would always crash due to a vector subscript getting out of range, whether I used a vector to compare the shapes
or physically coded each comparison (ie, shape 1 & shape 2, shape 1 & shape 3, and shape 2 & shape 3)
I'm not really sure where the problem is, why it is happening, or what is causing it
If I had more time, I would have asked you for help
But since I started and finished this last homework after the final presentation, that doesn't seem like an option
Regardless, here is what I have. Logically, it should work. In reality, it is hitting some weird error and crashing
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

// Vector class
// Made mostly because it was referenced to in the provided functions
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

// Modified entity class to work with this assignment
class Entity{
public:
	Entity(Vector position, Vector velocity, Vector size, float rotation, float scale, vector<float> coordinates) :
		position(position), velocity(velocity), size(size), rotation(rotation), scale(scale), coordinates(coordinates) {};

	Vector position;
	Vector velocity;
	Vector size;
	vector<float> coordinates;
	float rotation;
	float scale;
	Matrix modelMatrix;

	void Update(float elapsed) {
		position.x += velocity.x * elapsed;
		position.y += velocity.y * elapsed;
		if (position.x > 3.55f) {
			float penetration = fabs((position.x + size.x / 2) - 3.55f);
			position.x -= (penetration + 0.01);
			velocity.x *= -1;
		}
		else if (position.x < -3.55f) {
			float penetration = fabs((position.x + size.x / 2) + 3.55f);
			position.x += (penetration + 0.01);
			velocity.x *= -1;
		}
		if (position.y > 2.0f) {
			float penetration = fabs((position.y + size.y / 2) - 2.0f);
			position.y -= (penetration + 0.01);
			velocity.y *= -1;
		}
		else if (position.y < -2.0f) {
			float penetration = fabs((position.y + size.y / 2) + 2.0f);
			position.y += (penetration + 0.01);
			velocity.y *= -1;
		}
		rotation += elapsed;
	}

	void Draw(ShaderProgram *program) {
		float vertices[] = { -0.5f, -0.5f,
			0.5f, 0.5f,
			-0.5f, 0.5f,
			0.5f , 0.5f,
			-0.5f, -0.5f,
			0.5f, -0.5f };
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

// Function provided by Ivan Safrin
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

// Function provided by Ivan Safrin
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

// Function to convert from model coordinates to world coordinates
Vector convertToWorldCoordinates(Entity shape, Vector modelCoordinates) {
	Vector worldCoordinates(0.0f, 0.0f);
	worldCoordinates.x = (shape.modelMatrix.m[0][0] * modelCoordinates.x) + (shape.modelMatrix.m[1][0] * modelCoordinates.y) + shape.modelMatrix.m[3][0];
	worldCoordinates.y = (shape.modelMatrix.m[0][1] * modelCoordinates.x) + (shape.modelMatrix.m[1][1] * modelCoordinates.y) + shape.modelMatrix.m[3][1];
	return worldCoordinates;
}

// Function to check for a collision between 2 entities
bool checkCollision(Entity square, Entity rectangle) {
	vector<Vector> edges1;
	vector<Vector> edges2;
	for (int i = 0; i < square.coordinates.size(); i += 2) {
		Vector vec(square.coordinates[i], square.coordinates[i+1]);
		edges1.push_back(convertToWorldCoordinates(square, vec));
	}
	for (int i = 0; i < rectangle.coordinates.size(); i += 2) {
		Vector vec(rectangle.coordinates[i], rectangle.coordinates[i + 1]);
		edges1.push_back(convertToWorldCoordinates(rectangle, vec));
	}
	return(checkSATCollision(edges1, edges2));
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

	// Make the shapes
	vector<Entity> entities;
	Entity square(Vector(1.0f, 1.0f), Vector(1.0f, 1.0f), Vector(1.0f, 1.0f), 1.0f, 1.0f, { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f });
	Entity rectangle(Vector(-2.5f, 0.5f), Vector(-1.0f, 0.5f), Vector(1.0f, 1.0f), 0.0f, 1.0f, { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f });
	Entity anotherSquare(Vector(0.0f, 0.0f), Vector(0.25f, -3.0f), Vector(2.0f, 2.0f), 0.0f, 2.0f, { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f });

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

		// I could not get the collision function to work
		// whether I used the entities vector or I hard coded it to compare each shape to the others, it would always crash
		// Vector subscript would get out of range, not sure where, why, or how
		// If I had more time, I would have asked you for help with the code to find the problem
		// But since i'm submitting it so late, that isn't really an option

		/*
		for (int i = 0; i < entities.size() - 1; i++) {
			entities[i].Update(elapsed);
			for (int j = i + 1 ; j < entities.size(); j++) {
				int maxChecks = 10;
				while (checkCollision(entities[i], entities[j]) && maxChecks > 0) {
					Vector responseVector = Vector(entities[i].position.x - entities[j].position.x, entities[i].position.y - entities[j].position.y);
					responseVector.normalize();
					entities[i].position.x -= responseVector.x * 0.002;
					entities[i].position.y -= responseVector.y * 0.002;
					entities[j].position.x += responseVector.x * 0.002;
					entities[j].position.y += responseVector.y * 0.002;
					maxChecks--;
				}
			}
			entities[i].Draw(&program);
		}
		*/
		
		/*
		for (int i = 0; i < entities.size(); i++) {
			entities[i].Update(elapsed);
		}
		if (checkCollision(entities[0], entities[1])) {
			Vector responseVector = Vector(entities[0].position.x - entities[1].position.x, entities[0].position.y - entities[1].position.y);
			responseVector.normalize();
			entities[0].position.x -= responseVector.x * 0.002;
			entities[0].position.y -= responseVector.y * 0.002;
			entities[1].position.x += responseVector.x * 0.002;
			entities[1].position.y += responseVector.y * 0.002;
		}
		if (checkCollision(entities[0], entities[2]) && maxChecks > 0) {
		Vector responseVector = Vector(entities[0].position.x - entities[2].position.x, entities[0].position.y - entities[2].position.y);
		responseVector.normalize();
		entities[0].position.x -= responseVector.x * 0.002;
		entities[0].position.y -= responseVector.y * 0.002;
		entities[2].position.x += responseVector.x * 0.002;
		entities[2].position.y += responseVector.y * 0.002;
		maxChecks--;
		}
		if (checkCollision(entities[1], entities[2]) && maxChecks > 0) {
		Vector responseVector = Vector(entities[1].position.x - entities[2].position.x, entities[1].position.y - entities[2].position.y);
		responseVector.normalize();
		entities[1].position.x -= responseVector.x * 0.002;
		entities[1].position.y -= responseVector.y * 0.002;
		entities[2].position.x += responseVector.x * 0.002;
		entities[2].position.y += responseVector.y * 0.002;
		maxChecks--;
		}
		for (int i = 0; i < entities.size(); i++) {
			entities[i].Draw(&program);
		}
		*/

		// Update and draw
		for (int i = 0; i < entities.size(); i++) {
			entities[i].Update(elapsed);
			entities[i].Draw(&program);
		}

		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}