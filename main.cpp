#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 360;

void init();
void initGL();
void close();
void input();
void render();

SDL_Window *window = NULL;
SDL_GLContext glContext = NULL;
GLubyte glBuffer[WINDOW_HEIGHT][WINDOW_WIDTH];
bool quit = false;

int main() {
   std::cout << "Hello World";

   init();
   
   render(); // only render one frame
   do {
//      render();
      input();
   } while (!quit);
   
done:
   close();
}

/* initialise SDL */
void init() {
   SDL_Init(SDL_INIT_VIDEO);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
   window = SDL_CreateWindow("Path Tracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN); 
   initGL();
}

/* initialise GL */
void initGL() {
   glContext = SDL_GL_CreateContext(window);
   SDL_GL_SetSwapInterval(0);
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   glShadeModel(GL_FLAT);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

/* clean up */
void close() {
   SDL_DestroyWindow(window);
   window = NULL;
   SDL_Quit();
}

/* input */
void input() {
   SDL_Event e;
   while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
         quit = true;
      }
   }
}

/* main render routine */
void render() {
   int col, row;
   for (row = 0; row < WINDOW_HEIGHT; row++) {
      for (col = 0; col < WINDOW_WIDTH; col++) {
         glBuffer[row][col] = 0;
      }
   }

   /* refresh window buffer */
   glClear(GL_COLOR_BUFFER_BIT);
   glDrawPixels(WINDOW_WIDTH, WINDOW_HEIGHT, GL_LUMINANCE, GL_UNSIGNED_BYTE, glBuffer);
   SDL_GL_SwapWindow(window);
}
