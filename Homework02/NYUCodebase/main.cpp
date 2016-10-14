/*
-----QUESTIONS-----

1) None of the paddles are moving. Why? Is it my keyboard input? Or my velocity equation? Or my position equation?

2) How do I monitor collisions between the ball and the paddles? Should I like, keep tabs on the top and bottom of the paddles? or is there an easier way?

*/

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

#define CONVERT2RADIANS 3.14159265 / 180

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
	float rightPaddlePositionY = 0.0f;
	float ballPositionX = 0.0f;
	float ballPositionY = 0.0f;

	float leftPaddleVelocityY = 0.0f;
	float rightPaddleVelocityY = 0.0f;
	float ballVelocityX = 0.0f;
	float ballVelocityY = 0.0f;

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

		glClear(GL_COLOR_BUFFER_BIT);

		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		if (keys[SDL_SCANCODE_UP]) {
			leftPaddleVelocityY = 2.0;
		}
		else if (keys[SDL_SCANCODE_DOWN]) {
			leftPaddleVelocityY = -2.0;
		}
		else if (keys[SDL_SCANCODE_W]) {
			rightPaddleVelocityY = 2.0;
		}
		else if (keys[SDL_SCANCODE_S]) {
			rightPaddleVelocityY = -2.0;
		}

		float centerLineVertices[] = {0.05, 2.0, -0.05, 2.0, -0.05, -2.0, 0.05, 2.0, -0.05, -2.0, 0.05, -2.0};
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, centerLineVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);

		float leftPaddleVertices[] = {-3.05, 0.35, -3.30, 0.35, -3.30, -0.35, -3.05, 0.35, -3.30, -0.35, -3.05, -0.35};
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, leftPaddleVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);

		float rightPaddleVertices[] = { 3.05, 0.35, 3.30, 0.35, 3.30, -0.35, 3.05, 0.35, 3.30, -0.35, 3.05, -0.35 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, rightPaddleVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);

		/*

		float ballVertices[] = {};
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);

		*/

		leftPaddlePositionY += leftPaddleVelocityY * elapsed * sin(90 * CONVERT2RADIANS);
		rightPaddlePositionY += rightPaddleVelocityY * elapsed * sin(90 * CONVERT2RADIANS);

		leftPaddleModelMatrix.Translate(0, leftPaddlePositionY, 0);

		ballPositionX += ballVelocityX * elapsed;
		ballPositionY += ballVelocityY * elapsed;

		if (ballPositionX >= 3.55 || ballPositionX <= -3.55) {
			done = true;
		}

		if (ballPositionY >= 2.0 || ballPositionY <= -2.0) {
			ballVelocityY *= -1;
		}
		program.setModelMatrix(centerLineModelMatrix);
		program.setModelMatrix(leftPaddleModelMatrix);
		program.setModelMatrix(rightPaddleModelMatrix);
		program.setModelMatrix(ballModelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}