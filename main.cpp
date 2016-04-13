#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <Dense>
#include "math.h"

using namespace Eigen;

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 300;

void init();
void initGL();
void close();
void input();
void handleKey(SDL_Keycode k);
void render();

SDL_Window *window = NULL;
SDL_GLContext glContext = NULL;
GLfloat glBuffer[WINDOW_HEIGHT][WINDOW_WIDTH][3];
bool quit = false;

int samples = 1;
int inspect_x = -1;
int inspect_y = -1;
Scene scene = Scene();

int main() {
   init();
   scene.load_scene("cornell.txt");
   srand(time(NULL));

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
      } else if (e.type == SDL_MOUSEBUTTONDOWN ) {
         inspect_x = e.button.x;
         inspect_y = WINDOW_HEIGHT - e.button.y;
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
   Vector3d &pos = scene.cam.pos; //cam.pos;

   switch (k) {
      case SDLK_DOWN: {
                         pos(1)--;
                         samples = 1;
                         break;
                      }
      case SDLK_UP: {
                       pos(1)++;
                       samples = 1;
                       break;
                    }
      case SDLK_LEFT: {
                         pos(0)--;
                         samples = 1;
                         break;
                      }
      case SDLK_RIGHT: {
                          pos(0)++;
                          samples = 1;
                          break;
                       }
      case SDLK_w: {
                      pos(2)--;
                      samples = 1;
                      break;
                   }
      case SDLK_s: {
                      pos(2)++;
                      samples = 1;
                      break;
                   }
      case SDLK_q: {
                      pos(2)-=1e6;
                      samples = 1;
                      break;
                   }
      case SDLK_a: {
                      pos(2)+=1e6;
                      samples = 1;
                      break;
                   }
   }
}

inline Sphere* intersect(const Ray &r, double &t) {
   double d, inf = t = 1e20;
   Sphere* id = NULL;
   // for each mesh
   for (Sphere &sphere : scene.spheres) {
      if ((d = sphere.intersect(r)) && d < t) {
         t = d; id = &sphere;
      }
   }
   return id;
}

const Vector3d void_colour = Vector3d(0, 0, 0);

inline void fill_pixel(GLfloat pixel[3], const Vector3d &col) {
   pixel[0] = col(0);
   pixel[1] = col(1);
   pixel[2] = col(2);
}

inline void add_pixel(GLfloat pixel[3], const Vector3d &col) {
   double n = (double)1/(samples);
   n += 1/5;
   //n = (n < 0.07) ? 0.05 : n;
   double o = 1 - n ;//1.0 - n;
   pixel[0] = o * pixel[0] + n * clamp(col(0));
   pixel[1] = o * pixel[1] + n * clamp(col(1));
   pixel[2] = o * pixel[2] + n * clamp(col(2));
}

inline double randRange(double low, double high) { 
   double range = high - low;
   return low + (double) rand() / (RAND_MAX / range);
}

inline double unitRand() {
   return randRange(0, 1);
}

inline Vector3d unitSphereRand() {

   double x1, x2;
   double x1s, x2s;
   do {
      x1 = randRange(-1, 1); 
      x2 = randRange(-1, 1);
      x1s = x1 * x1;
      x2s = x2 * x2;
   } while (x1s + x2s >= 1);
   return Vector3d(
         2 * x1 * sqrt(1 - x1s - x2s),
         2 * x2 * sqrt(1 - x1s - x2s),
         1 - 2 * (x1s + x2s)); 
   // double theta0 = 2 * 3.1415926535 * unitRand();
   // double theta1 = acos(1 - 2 * unitRand());
   // return Vector3d(
   //       sin(theta0) * sin(theta1),
   //       sin(theta0) * cos(theta1),
   //       sin(theta1)
   //       );
}

double lambert(Vector3d hit, Vector3d n) {
   double d;
   double i = 0.0;
   for (Sphere &light : scene.spheres) {
      if (light.mat.emit <= 0.0) continue;

      double t;
      Sphere* id = NULL;
      // random point on light
      Vector3d light_pos = light.pos + unitSphereRand() * light.rad;

      Vector3d l = (light_pos - hit).normalized();
      if (&light == intersect(Ray(hit, l), t)) {
         i += light.mat.emit * n.dot(l);        
      }
   }
   return i;
}

const int depth_max = 6;
const int depth_direct = 4;
const int depth_reflection = 3; 
const int depth_refractions = 1;

Vector3d trace(Ray r, int depth = 0, bool debug = false) {
   // only debug first 2 bounces
   //debug = debug && depth < 2;

   Vector3d final_colour = void_colour;
   double heat = 1.0;
   
   if (debug && depth == 0) {
      std::cout << std::endl << std::endl;
      std::cout << "NEW RAY " << inspect_x << " " << inspect_y << std::endl;
   }

   double t;
   Sphere* id = NULL;

   // if we hit something
   if (NULL != (id = intersect(r, t))) {
      // hit information
      Vector3d hit = r.origin + r.dir * t;
      Vector3d n = (hit - id->pos).normalized();
      Vector3d refl_dir = r.dir - (n * (2 * r.dir.dot(n))); 
      Vector3d rand_dir = unitSphereRand();

      // material information
      Material mat = id->mat;
      Vector3d col = mat.col;
      double diff = mat.diff;
      double emit = mat.emit;

      if (debug) std::cout << "hit object: "
         << mat.name << std::endl
         << mat.diff << " "
         << mat.refl << " "
         << mat.emit << " "
         << std::endl;

      // exceed ray bounce depth
      if (depth > depth_max) {
         if (debug) std::cout << "END: exceeded max depth" << std::endl;
         return col * diff * lambert(hit, n);
      }

      // treat lights special for now
      if (emit > 0.0) {
         if (debug) std::cout << "END: hit light" << std::endl;
         return emit * col;
      }

      // reflection ray 
      if (depth < depth_reflection) {
         // reflection ray
         double refr_val = mat.refl;
         if (refr_val > 0.0) {
            Ray refl_ray = Ray(hit, refl_dir);

            final_colour += refr_val * trace(refl_ray, depth + 1, debug);
            heat -= refr_val;
         }
      }

      // refraction ray

      // direct illumination
      if (depth < depth_direct) {
         // calculate direct lighting
         double light_int = lambert(hit, n);
         // DIRECT COMPONENT ??
         final_colour += col * diff * light_int;
         heat -= light_int;
         if (heat < 0) heat = 0;
      }

      // ambient ray direction
      Vector3d new_dir = rand_dir;
      //if (depth > 1) {
      //   rand_dir = (0.8 * refl_dir + 0.2 * rand_dir).normalized(); 
      //} else 
      if (n.dot(new_dir) < 0) { // keep random ray
         new_dir *= -1;
      }
      Ray next_ray = Ray(hit, new_dir);

      Vector3d c = trace(next_ray, depth + 1, debug);

      // if we hit the void
      if (c == void_colour){
         return void_colour;
         //return col * diff * lambert(hit, n);
      } else {
         // REMAINING HEAT
         double amb = 0.2;
         return final_colour + heat * (((1 - amb) * diff * col) + (amb *  c)); 
      }
   }
}

/* main render routine */
void render() {
   int x, y;
   for (y = 0; y < WINDOW_HEIGHT; y++) {
      for (x = 0; x < WINDOW_WIDTH; x++) {
         Vector3d col = Vector3d(0, 0, 0);
         for (int i = 0; i < 2; i++) {
         for (int j = 0; j < 2; j++) {
            // create camera ray
            Vector3d d = cx*((.25 * i + x)/WINDOW_WIDTH - .5) +
               cy*((.25 * j + y)/WINDOW_HEIGHT - .5) + scene.cam.dir;
            Ray r = Ray(scene.cam.pos + d*140, d.normalized());

             col += trace(r, 0, x == inspect_x && y == inspect_y);
         }
         }
         add_pixel(glBuffer[y][x], col / 4);
      }
   }
   // finished one frame
   samples++;
   //std::cout << "samples: " << samples << std::endl;
   fflush(stdout);

   /* refresh window buffer */
   glClear(GL_COLOR_BUFFER_BIT);
   glDrawPixels(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_FLOAT, glBuffer);
   SDL_GL_SwapWindow(window);
}
