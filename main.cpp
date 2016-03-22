#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <Dense>

using namespace Eigen;

const int WINDOW_WIDTH = 256;
const int WINDOW_HEIGHT = 192;

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
   clock_t t1, t2;
   t1 = clock();
   do {
      render();
      input();

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

#include "math.cpp"

Material mat_white = Material(Vector3d(1, 1, 1), Vector3d(), 1.0, 0.0, 0.0);
Material mat_red   = Material(Vector3d(.75, .25, .25), Vector3d(), 1.0, 0.0, 0.0);
Material mat_blue  = Material(Vector3d(.25, .25, .75), Vector3d(), 1.0, 0.0, 0.0);

Material mat_light = Material(Vector3d(1, 1, 1), Vector3d(1, 1, 1), 1.0, 0.0, 0.0);
Material mat_glass = Material(Vector3d(1, 1, 1), Vector3d(0, 0, 0), 0.0, 0.0, 1.0);
Material mat_mirr = Material(Vector3d(1, 1, 1), Vector3d(0, 0, 0), 0.0, 1.0, 0.0);

/* scene data */
Sphere spheres[] = {
   Sphere(1.5, Vector3d(50,81.6-16.5,81.6), mat_light),//Lite
   Sphere(1e5, Vector3d(1e5+1,40.8,81.6), mat_red),//Left
   Sphere(1e5, Vector3d(-1e5+99,40.8,81.6),mat_blue),//Rght
   Sphere(1e5, Vector3d(50,40.8, 1e5),     mat_white),//Back
   Sphere(1e5, Vector3d(50,40.8,-1e5+170), mat_white), //Frnt
   Sphere(1e5, Vector3d(50, 1e5, 81.6), mat_white),//Botm
   Sphere(1e5, Vector3d(50,-1e5+81.6,81.6), mat_white),//Top
   Sphere(16.5,Vector3d(57,16.5,37), mat_white),//Mirr
   Sphere(16.5,Vector3d(27,16.5,47), mat_mirr), //Mirr
   Sphere(16.5,Vector3d(73,16.5,80), mat_glass),//Glas
};
int numSpheres = sizeof(spheres)/sizeof(Sphere);
Ray cam(Vector3d(50, 52, 295.6), Vector3d(0, -0.042612, -1).normalized());
Vector3d cx = Vector3d(WINDOW_WIDTH*.5135/WINDOW_HEIGHT, 0, 0);
Vector3d cy = cx.cross(cam.dir).normalized() * 0.5135;

void handleKey(SDL_Keycode k) {
   Vector3d &pos = spheres[9].pos; //cam.origin;

   switch (k) {
      case SDLK_DOWN: {
         pos(1)--;
         break;
      }
      case SDLK_UP: {
         pos(2)++;
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



const Vector3d void_colour = Vector3d(1.0, 0.0, 1.0);

inline void fill_pixel(GLubyte pixel[3], const Vector3d &col) {
   pixel[0] = toInt(col(0)); // RED
   pixel[1] = toInt(col(1)); // GREEN
   pixel[2] = toInt(col(2)); // BLUE
}

Vector3d trace(Ray r, int depth) {
   Vector3d col = void_colour;

   double t;
   int id = 0;
   if (intersect(r, t, id)) {
      Material mat = spheres[id].mat;
      Sphere light = spheres[0];

      Vector3d col_diff = mat.col;
      Vector3d col_refr = Vector3d();
      Vector3d col_trans = Vector3d();

      // hit point data
      Vector3d hit = r.origin + r.dir * t;
      Vector3d n = (hit - spheres[id].pos).normalized();
      Vector3d l = (light.pos - hit).normalized();
      double lambert = clamp(n.dot(l));

      // reflection ray
      double refl_val = mat.refl;
      if (depth > 0 && refl_val > 0.0) {
         Vector3d refl_dir = r.dir - (n * (2 * r.dir.dot(n))); 
         Ray refl_ray = Ray(hit, refl_dir);
         col_refr = trace(refl_ray, depth - 1);
      }

      // refractive ray
      double refr_val = mat.trans;
      if (depth > 0 && refr_val > 0) {
         //   double test = 1 / n.dot(r.dir);
         //   Vector3d trans_dir = r.dir + n * test;
         //   trans_dir = trans_dir.normalized();

         //     Vector3d c = (n % r.dir).norm();
         //     double theta = atan(c.y / c.x);
         //     double phi = acos(c.z);
         Vector3d axis = n.cross(r.dir);
         AngleAxisd t = AngleAxisd(0.1, axis); 
         Vector3d trans_dir = t.toRotationMatrix() * r.dir;
         Ray trans_ray = Ray(hit, trans_dir.normalized());
         col_trans = trace(trans_ray, depth); 
      }

      // diffuse
      Ray shadow = Ray(hit, l); 
      if (intersect(shadow, t, id)) {
         if (0 == id) { // if we hit light
            col_diff = col_diff * lambert;
         } else {
            col_diff = Vector3d(0, 0, 0);
         } 
      }
      col = col_diff * lambert * mat.diff + col_refr * mat.refl + col_trans * mat.trans;
   }
   return col;
}

/* main render routine */
void render() {
   int x, y;
   for (y = 0; y < WINDOW_HEIGHT; y++) {
      for (x = 0; x < WINDOW_WIDTH; x++) {
         // create camera ray
         Vector3d d = cx*(((0 + .5 + 0)/2 + x)/WINDOW_WIDTH - .5) +
            cy*(((0 + .5 + 0)/2 + y)/WINDOW_HEIGHT - .5) + cam.dir;
         Ray r = Ray(cam.origin + d*140, d.normalized());
         fill_pixel(glBuffer[y][x], trace(r, 5));
      }
   }

   /* refresh window buffer */
   glClear(GL_COLOR_BUFFER_BIT);
   glDrawPixels(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, glBuffer);
   SDL_GL_SwapWindow(window);
}
