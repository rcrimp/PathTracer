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
      //fprintf(stderr, "%f seconds\n", (float)(t2-t1) / CLOCKS_PER_SEC);
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
   // return int(pow(clamp(x), 1/2.2)*255+.5);
   return int(clamp(x)*255);
}

//Material mat_white = Material("white", Vector3d(1, 1, 1), 1.0, 0.0, 0.0);
//Material mat_red   = Material("red", Vector3d(.75, .25, .25), 1.0, 0.0, 0.0);
//Material mat_blue  = Material("blue", Vector3d(.25, .25, .75), 1.0, 0.0, 0.0);
//
//Material mat_light = Material("light", Vector3d(1, 1, 1), 1.0, 0.0, 0.0);
//Material mat_glass = Material("glass", Vector3d(1, 1, 1), 0.0, 0.0, 1.0);
//Material mat_mirr = Material("mirror", Vector3d(1, 1, 1), 0.0, 1.0, 0.0);
//
///* scene data */
//Sphere spheres[] = {
//   Sphere(Vector3d(50,81.6-16.5,81.6), 1.5, mat_light),//Lite
//   Sphere(Vector3d(1e5+1,40.8,81.6),   1e5, mat_red),//Left
//   Sphere(Vector3d(-1e5+99,40.8,81.6), 1e5, mat_blue),//Rght
//   Sphere(Vector3d(50,40.8, 1e5),      1e5, mat_white),//Back
//   Sphere(Vector3d(50,40.8,-1e5+170),  1e5, mat_white), //Frnt
//   Sphere(Vector3d(50, 1e5, 81.6),     1e5, mat_white),//Botm
//   Sphere(Vector3d(50,-1e5+81.6,81.6), 1e5, mat_white),//Top
//   Sphere(Vector3d(57,16.5,37),        16.5,mat_white),//Mirr
//   Sphere(Vector3d(27,16.5,47),        16.5,mat_mirr), //Mirr
//   Sphere(Vector3d(73,16.5,80),        16.5,mat_glass),//Glas
//};
//int numSpheres = sizeof(spheres)/sizeof(Sphere);
//
//Camera cam = Camera(Vector3d(50, 52, 295.6), Vector3d(0, -0.042612, -1).normalized(), 45.0);


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

const Vector3d void_colour = Vector3d(0.0, 0.0, 0.0);

inline void fill_pixel(GLubyte pixel[3], const Vector3d &col) {
   pixel[0] = toInt(col(0));// < 1 ? col(0) : 1); // RED
   pixel[1] = toInt(col(1));// < 1 ? col(1) : 1); // GREEN
   pixel[2] = toInt(col(2));// < 1 ? col(2) : 1); // BLUE
}

inline double unitRand() {
   return (double) rand() / (RAND_MAX);
}


Vector3d trace(Ray r, int depth) {
   Vector3d col = void_colour;

   if (depth == 0) {
      return void_colour; 
   }

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

      Ray rand_ray = Ray(hit, rand_dir);
      
      double k = 0.5; // decay term
      
      Vector3d c = trace(rand_ray, depth - 1);
      if (c(0) == 0 && c(1) == 0 && c(2) == 0){ // if black
         return void_colour; 
      } else {
         return (diff * col) + (k *  c); 
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

         int samples = 20;
         Vector3d col = Vector3d(0, 0, 0);
         for (int i = 0; i < samples; i++){
            col += 0.05 * trace(r, 10);
         }
         fill_pixel(glBuffer[y][x], col);
      }
   }

   /* refresh window buffer */
   glClear(GL_COLOR_BUFFER_BIT);
   glDrawPixels(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, glBuffer);
   SDL_GL_SwapWindow(window);
}
