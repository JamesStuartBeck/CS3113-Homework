// Weird collision stuff going on, but pretty much done

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "ShaderProgram.h"
#include "Matrix.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define CONVERT2RADIANS 3.14159265 / 180.0
#define RAND_MAX = 360;

SDL_Window* displayWindow;

GLuint LoadTexture(const char *image_path) {
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

bool collision(float box1PositionX, float box1PositionY, float box1Width, float box1Height, float box2PositionX, float box2PositionY, float box2Width, float box2Height) {
	if ((box1PositionY - (box1Height/2) > box2PositionY + (box2Height/2)) || (box1PositionY + (box1Height/2) < box2PositionY - (box2Height/2)) || (box1PositionX - (box1Width/2) > box2PositionX + (box2Width/2)) || (box1PositionX + (box1Width/2) < box2PositionX - (box2Width/2))) {
		return false;
	}
	else {
		return true;
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

	ShaderProgram program(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

	Matrix projectionMatrix;
	Matrix centerLineModelMatrix;
	Matrix leftPaddleModelMatrix;
	Matrix rightPaddleModelMatrix;
	Matrix ballModelMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	glUseProgram(program.programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float leftPaddlePositionY = 0.0f;
	float leftPaddleHeight = 0.7f;
	float leftPaddleWidth = 0.25f;
	float rightPaddlePositionY = 0.0f;
	float rightPaddleHeight = 0.7f;
	float rightPaddleWidth = 0.25f;
	float ballPositionX = 0.0f;
	float ballPositionY = 0.0f;
	float ballHeight = 0.1f;
	float ballWidth = 0.1f;

	float leftPaddleVelocityY = 0.0f;
	float rightPaddleVelocityY = 0.0f;
	float ballVelocityX = 2.0f;
	float ballVelocityY = 2.0f;
	int ballAngle = rand();

	float lastFrameTicks = 0.0f;

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		leftPaddlePositionY += leftPaddleVelocityY * elapsed;
		rightPaddlePositionY += rightPaddleVelocityY * elapsed;

		ballPositionX += ballVelocityX * elapsed * cos(ballAngle * CONVERT2RADIANS);
		ballPositionY += ballVelocityY * elapsed * sin(ballAngle * CONVERT2RADIANS);

		if (ballPositionX > 3.55 || ballPositionX < -3.55) {
			done = true;
		}

		if (ballPositionY >= 2.0f) {
			if (ballVelocityX > 0.0f) {
				ballAngle += 90.0f;
			}
			else {
				ballAngle -= 90.0f;
			}
		}
		else if (ballPositionY <= -2.0f) {
			if (ballVelocityX > 0.0f) {
				ballAngle += 90.0f;
			}
			else {
				ballAngle -= 90.0f;
			}
		}

		if (collision(-3.175f, leftPaddlePositionY, leftPaddleWidth, leftPaddleHeight, ballPositionX, ballPositionY, ballWidth, ballHeight)) {
		ballAngle += 180.0f;
		}
		else if (collision(3.175f, rightPaddlePositionY, rightPaddleWidth, rightPaddleHeight, ballPositionX, ballPositionY, ballWidth, ballHeight)) {
		ballAngle += 180.0f;
		}

		glClear(GL_COLOR_BUFFER_BIT);

		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		leftPaddleVelocityY = 0.0f;
		rightPaddleVelocityY = 0.0f;

		if (keys[SDL_SCANCODE_UP]) {
			rightPaddleVelocityY = 2.0f;
		}
		else if (keys[SDL_SCANCODE_DOWN]) {
			rightPaddleVelocityY = -2.0f;
		}
		if (keys[SDL_SCANCODE_W]) {
			leftPaddleVelocityY = 2.0f;
		}
		else if (keys[SDL_SCANCODE_S]) {
			leftPaddleVelocityY = -2.0f;
		}

		if ((leftPaddlePositionY + (leftPaddleHeight / 2)) > 2.0f) {
			leftPaddleVelocityY = 0.0f;
			leftPaddlePositionY = 1.99 - leftPaddleHeight/2;
		}
		else if ((leftPaddlePositionY - (leftPaddleHeight / 2)) < -2.0f) {
			leftPaddleVelocityY = 0.0f;
			leftPaddlePositionY = -1.99 + leftPaddleHeight/2;
		}

		if ((rightPaddlePositionY + (rightPaddleHeight / 2)) > 2.0f) {
			rightPaddleVelocityY = 0.0f;
			rightPaddlePositionY = 1.99 - rightPaddleHeight / 2;
		}
		else if ((rightPaddlePositionY - (rightPaddleHeight / 2)) < -2.0f) {
			rightPaddleVelocityY = 0.0f;
			rightPaddlePositionY = -1.99 + rightPaddleHeight / 2;
		}

		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		float centerLineVertices[] = {0.05, 2.0, -0.05, 2.0, -0.05, -2.0, 0.05, 2.0, -0.05, -2.0, 0.05, -2.0};
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, centerLineVertices);
		glEnableVertexAttribArray(program.positionAttribute);
		program.setModelMatrix(centerLineModelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);

		float leftPaddleVertices[] = {-3.05, 0.35, -3.30, 0.35, -3.30, -0.35, -3.05, 0.35, -3.30, -0.35, -3.05, -0.35};
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, leftPaddleVertices);
		glEnableVertexAttribArray(program.positionAttribute);
		leftPaddleModelMatrix.identity();
		leftPaddleModelMatrix.Translate(0, leftPaddlePositionY, 0);
		program.setModelMatrix(leftPaddleModelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);

		float rightPaddleVertices[] = { 3.05, 0.35, 3.30, 0.35, 3.30, -0.35, 3.05, 0.35, 3.30, -0.35, 3.05, -0.35 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, rightPaddleVertices);
		glEnableVertexAttribArray(program.positionAttribute);
		rightPaddleModelMatrix.identity();
		rightPaddleModelMatrix.Translate(0, rightPaddlePositionY, 0);
		program.setModelMatrix(rightPaddleModelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);

		float ballVertices[] = {0.05, 0.05, -0.05, 0.05, -0.05, -0.05, 0.05, 0.05, -0.05, -0.05, 0.05, -0.05};
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballVertices);
		glEnableVertexAttribArray(program.positionAttribute);
		ballModelMatrix.identity();
		ballModelMatrix.Translate(ballPositionX, ballPositionY, 0);
		program.setModelMatrix(ballModelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}