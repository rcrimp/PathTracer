#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <Dense>
#include "math.h"

using namespace Eigen;

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 320;

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

int samples = 1;
Scene scene = Scene();

int main() {
   init();
   scene.load_scene("cornell.txt");

   clock_t t1, t2;
   t1 = clock();
   do {
      render();
      input();
      //   getc(stdin);

      t2 = clock();
      fprintf(stderr, "%f seconds\n", (float)(t2-t1) / CLOCKS_PER_SEC);
      t1 = t2;
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

inline double clamp(double x) {
   return (x < 0) ? 0 : (x > 1) ? 1 : x;
}

inline int toInt(double x) {
   //return int(pow(clamp(x), 1/2.2)*255+.5);
   return int(clamp(x)*255);
}

Vector3d cx = Vector3d(WINDOW_WIDTH*.5135/WINDOW_HEIGHT, 0, 0);
Vector3d cy = cx.cross(scene.cam.dir).normalized() * 0.5135;

void handleKey(SDL_Keycode k) {
   Vector3d &pos = scene.spheres.front().pos; //cam.pos;

   switch (k) {
      case SDLK_DOWN: {
                         pos(1)--;
                         break;
                      }
      case SDLK_UP: {
                       pos(1)++;
                       break;
                    }
      case SDLK_LEFT: {
                         pos(0)--;
                         break;
                      }
      case SDLK_RIGHT: {
                          pos(0)++;
                          break;
                       }
      case SDLK_w: {
                      pos(2)--;
                      break;
                   }
      case SDLK_s: {
                      pos(2)++;
                      break;
                   }
      case SDLK_q: {
                      pos(2)-=1e6;
                      break;
                   }
      case SDLK_a: {
                      pos(2)+=1e6;
                      break;
                   }
   }
   std::cout << pos << std::endl;
   samples = 1;
}

inline Sphere* intersect(const Ray &r, double &t) {
   double d, inf = t = 1e20;
   Sphere* id = NULL;
   // for each mesh
   for (Sphere &sphere : scene.spheres) {
      //std::cout << sphere.mat.name << " " ;
      //fflush(stdout);
      if ((d = sphere.intersect(r)) && d < t) {
         t = d; id = &sphere;
      }
   }
   //std::cout << std::endl;
   return id;
}

const Vector3d void_colour = Vector3d(0, 0, 0);

inline void fill_pixel(GLubyte pixel[3], const Vector3d &col) {
   pixel[0] = toInt(col(0));// < 1 ? col(0) : 1); // RED
   pixel[1] = toInt(col(1));// < 1 ? col(1) : 1); // GREEN
   pixel[2] = toInt(col(2));// < 1 ? col(2) : 1); // BLUE
}

inline void add_pixel(GLubyte pixel[3], const Vector3d &col) {
   double n = (double)1/samples;
   double o = 1.0 - n;
   pixel[0] = o * pixel[0] + toInt(n * col(0));// < 1 ? col(0) : 1); // RED
   pixel[1] = o * pixel[1] + toInt(n * col(1));// < 1 ? col(1) : 1); // GREEN
   pixel[2] = o * pixel[2] + toInt(n * col(2));// < 1 ? col(2) : 1); // BLUE
}

inline double unitRand() {
   return (double) rand() / (RAND_MAX);
}

Vector3d trace(Ray r, int depth) {
   Vector3d col = void_colour;

   // base case
   if (depth == 0) {
      return void_colour; 
   }

   // recursive case
   double t;
   Sphere* id = NULL;
   if (NULL != (id = intersect(r, t))) {
      // hit information
      Vector3d hit = r.origin + r.dir * t;
      Vector3d n = (hit - id->pos).normalized();

      // material information
      Material mat = id->mat;
      Vector3d col = mat.col;
      double diff = mat.diff;
      double emit = mat.emit;

      if (emit > 0.0) {
         return emit * col;
      }
      
      // random ray direction
      double theta0 = 2 * 3.1415926535 * unitRand();
      double theta1 = acos(1 - 2 * unitRand());
      Vector3d rand_dir = Vector3d(
            sin(theta0) * sin(theta1),
            sin(theta0) * cos(theta1),
            sin(theta1)
            );
      if (n.dot(rand_dir) < 0) {
         rand_dir *= -1;
      }

      Ray rand_ray = Ray(hit, rand_dir);
      
      double k1 = 0.3;
      double k2 = 1 - k1;
      
      Vector3d c = trace(rand_ray, depth - 1);
      // if the recursive ray DOESN'T hits a light
      if (c == void_colour){
         return Vector3d(0, 0, 0); 
      } else { // else we never hit a light
         return (k2 * diff * col) + (k1 *  c); 
      }
   }
   return col;
}

/* main render routine */
void render() {
   int x, y;
   for (y = 0; y < WINDOW_HEIGHT; y++) {
      for (x = 0; x < WINDOW_WIDTH; x++) {
         // create camera ray
         Vector3d d = cx*((.5 + x)/WINDOW_WIDTH - .5) +
            cy*((.5 + y)/WINDOW_HEIGHT - .5) + scene.cam.dir;
         Ray r = Ray(scene.cam.pos + d*140, d.normalized());

         Vector3d col = trace(r, 5);
         add_pixel(glBuffer[y][x], col);
         //fill_pixel(glBuffer[y][x], col);
      }
   }
   // finished one frame
   samples++;
   std::cout << "samples: " << samples << std::endl;
   fflush(stdout);

   /* refresh window buffer */
   glClear(GL_COLOR_BUFFER_BIT);
   glDrawPixels(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, glBuffer);
   SDL_GL_SwapWindow(window);
}
