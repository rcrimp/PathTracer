#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

void init();
void initGL();
void close();
void input();
void handleKey(SDL_Keycode k);
void render();

SDL_Window *window = NULL;
SDL_GLContext glContext = NULL;
GLubyte glBuffer[WINDOW_HEIGHT][WINDOW_WIDTH][3];
bool quit = false;

int main() {
   init();
   do {
      render();
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
      } else if (e.type == SDL_KEYDOWN) {
         handleKey(e.key.keysym.sym); 
      }
   }
}

#include "math.cpp"

/* scene data */
Sphere spheres[] = {
   Sphere(1.5, Vec(50,81.6-16.5,81.6),Vec(4,4,4)*100,  Vec()),//Lite
   Sphere(1e5, Vec( 1e5+1,40.8,81.6), Vec(),Vec(.75,.25,.25)),//Left
   Sphere(1e5, Vec(-1e5+99,40.8,81.6),Vec(),Vec(.25,.25,.75)),//Rght
   Sphere(1e5, Vec(50,40.8, 1e5),     Vec(),Vec(.75,.75,.75)),//Back
   Sphere(1e5, Vec(50,40.8,-1e5+170), Vec(),Vec()          ),//Frnt
   Sphere(1e5, Vec(50, 1e5, 81.6),    Vec(),Vec(.75,.75,.75)),//Botm
   Sphere(1e5, Vec(50,-1e5+81.6,81.6),Vec(),Vec(.75,.75,.75)),//Top
   Sphere(16.5,Vec(27,16.5,47),       Vec(),Vec(1,1,1)*.999),//Mirr
   Sphere(16.5,Vec(73,16.5,78),       Vec(),Vec(1,1,1)*.999),//Glas
};
int numSpheres = sizeof(spheres)/sizeof(Sphere);
Ray cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm());
Vec cx = Vec(WINDOW_WIDTH*.5135/WINDOW_HEIGHT);
Vec cy = (cx%cam.dir).norm()*.5135;

void handleKey(SDL_Keycode k) {
   switch (k) {
      case SDLK_DOWN: {
         cam.origin.y--;
         break;
      }
      case SDLK_UP: {
         cam.origin.y++;
         break;
      }
      case SDLK_LEFT: {
         cam.origin.x--;
         break;
      }
      case SDLK_RIGHT: {
         cam.origin.x++;
         break;
      }
      case SDLK_w: {
         cam.origin.z--;
         break;
      }
      case SDLK_s: {
         cam.origin.z++;
         break;
      }
      case SDLK_q: {
         cam.origin.z-=1e6;
         break;
      }
      case SDLK_a: {
         cam.origin.z+=1e6;
         break;
      }
   }
}

inline bool intersect(const Ray &r, double &t, int &id) {
   double d, inf = t = 1e20;
   for (int i = numSpheres;i--;) {
      if ((d = spheres[i].intersect(r)) && d < t) {
         t = d; id = i;
      }
   }
   return t < inf;
}



const Vec void_colour = Vec(1.0, 0.0, 1.0);
inline void fill_pixel(GLubyte pixel[3], const Vec &col) {
   pixel[0] = toInt(col.x); // RED
   pixel[1] = toInt(col.y); // GREEN
   pixel[2] = toInt(col.z); // BLUE

}

/* main render routine */
void render() {
   int x, y;
   for (y = 0; y < WINDOW_HEIGHT; y++) {
      for (x = 0; x < WINDOW_WIDTH; x++) {

         

         Vec d = cx*(((0 + .5 + 0)/2 + x)/WINDOW_WIDTH - .5) +
            cy*(((0 + .5 + 0)/2 + y)/WINDOW_HEIGHT - .5) + cam.dir;
         Ray r = Ray(cam.origin + d*140, d.norm());
         double t;
         int id = 0;

         Vec col = void_colour;
         if (intersect(r, t, id)) {
            col = spheres[id].col;

            Sphere light = spheres[0];

            Vec hit = r.origin + r.dir * t;
            Vec n = (hit - spheres[id].pos).norm();
            Vec l = (light.pos - hit).norm();
            double lambert = clamp(n.dot(l));


            //shadow ray
            Ray shadow = Ray(hit, (light.pos - hit).norm());
            if (intersect(shadow, t, id)) {
               if (0 == id) { // if we hit light
                  col.x *= lambert;
                  col.y *= lambert;
                  col.z *= lambert;
               } else {
                  col.x = 0;
                  col.y = 0;
                  col.z = 0;
               } 
            }
         }

         fill_pixel(glBuffer[y][x], col);
      }
   }

   /* refresh window buffer */
   glClear(GL_COLOR_BUFFER_BIT);
   glDrawPixels(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, glBuffer);
   SDL_GL_SwapWindow(window);
}
