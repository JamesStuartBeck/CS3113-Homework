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

	Matrix projectionMatrix;
	Matrix modelMatrixKing;
	Matrix modelMatrixQueen;
	Matrix modelMatrixJack;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	GLuint kingTexture = LoadTexture("cardSpadesK.png");
	GLuint queenTexture = LoadTexture("cardSpadesQ.png");
	GLuint jackTexture = LoadTexture("cardSpadesJ.png");

	glUseProgram(program.programID);

	float lastFrameTicks = 0.0f;

	float kingPosition = 0.0f;

	float jackPosition = 0.0f;

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);

		program.setModelMatrix(modelMatrixKing);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glBindTexture(GL_TEXTURE_2D, kingTexture);

		float verticesKing[] = { -0.5, -0.6, 0.5, -0.6, 0.5, 0.6, -0.5, -0.6, 0.5, 0.6, -0.5, 0.6 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesKing);
		glEnableVertexAttribArray(program.positionAttribute);

		float texCoordsKing[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordsKing);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		program.setModelMatrix(modelMatrixQueen);

		glBindTexture(GL_TEXTURE_2D, queenTexture);

		float verticesQueen[] = { -0.5, -0.6, 0.5, -0.6, 0.5, 0.6, -0.5, -0.6, 0.5, 0.6, -0.5, 0.6 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesQueen);
		glEnableVertexAttribArray(program.positionAttribute);

		float texCoordsQueen[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordsQueen);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		program.setModelMatrix(modelMatrixJack);

		glBindTexture(GL_TEXTURE_2D, jackTexture);

		float verticesJack[] = { -0.5, -0.6, 0.5, -0.6, 0.5, 0.6, -0.5, -0.6, 0.5, 0.6, -0.5, 0.6 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesJack);
		glEnableVertexAttribArray(program.positionAttribute);

		float texCoordsJack[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordsJack);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		if (kingPosition <= 0.5) {
			modelMatrixKing.Translate((ticks * 0.0001), 0.0, 0.0);
			kingPosition += 0.0001;
		}

		if (jackPosition >= -0.5) {
			modelMatrixJack.Translate((ticks * -0.0001), 0.0, 0.0);
			jackPosition -= 0.0001;
		}
		
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}