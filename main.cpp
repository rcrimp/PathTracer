#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

const int WINDOW_WIDTH = 512;
const int WINDOW_HEIGHT = 384;

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

Material mat_white = Material(Vec(1, 1, 1), Vec(), 1.0, 0.0, 0.0);
Material mat_red   = Material(Vec(.75, .25, .25), Vec(), 1.0, 0.0, 0.0);
Material mat_blue  = Material(Vec(.25, .25, .75), Vec(), 1.0, 0.0, 0.0);

Material mat_light = Material(Vec(1, 1, 1), Vec(1, 1, 1), 1.0, 0.0, 0.0);
Material mat_glass = Material(Vec(1, 1, 1), Vec(), 0.0, 0.5, 0.5);
Material mat_mirr = Material(Vec(1, 1, 1), Vec(), 0.0, 1.0, 0.0);

/* scene data */
Sphere spheres[] = {
   Sphere(1.5, Vec(50,81.6-16.5,81.6), mat_light),//Lite
   Sphere(1e5, Vec(1e5+1,40.8,81.6), mat_red),//Left
   Sphere(1e5, Vec(-1e5+99,40.8,81.6),mat_blue),//Rght
   Sphere(1e5, Vec(50,40.8, 1e5),     mat_white),//Back
   Sphere(1e5, Vec(50,40.8,-1e5+170), mat_white), //Frnt
   Sphere(1e5, Vec(50, 1e5, 81.6), mat_white),//Botm
   Sphere(1e5, Vec(50,-1e5+81.6,81.6), mat_white),//Top
   Sphere(16.5,Vec(57,16.5,37), mat_white),//Mirr
   Sphere(16.5,Vec(27,16.5,47), mat_mirr),//Mirr
   Sphere(16.5,Vec(73,16.5,78), mat_glass),//Glas
};
int numSpheres = sizeof(spheres)/sizeof(Sphere);
Ray cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm());
Vec cx = Vec(WINDOW_WIDTH*.5135/WINDOW_HEIGHT);
Vec cy = (cx%cam.dir).norm()*.5135;

void handleKey(SDL_Keycode k) {

   Vec &pos = spheres[9].pos; //cam.origin;

   switch (k) {
      case SDLK_DOWN: {
         pos.y--;
         break;
      }
      case SDLK_UP: {
         pos.y++;
         break;
      }
      case SDLK_LEFT: {
         pos.x--;
         break;
      }
      case SDLK_RIGHT: {
         pos.x++;
         break;
      }
      case SDLK_w: {
         pos.z--;
         break;
      }
      case SDLK_s: {
         pos.z++;
         break;
      }
      case SDLK_q: {
         pos.z-=1e6;
         break;
      }
      case SDLK_a: {
         pos.z+=1e6;
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

Vec trace(Ray r, int depth) {
   Vec col = void_colour;

   double t;
   int id = 0;
   if (intersect(r, t, id)) {
      Material mat = spheres[id].mat;
      Sphere light = spheres[0];

      Vec col_diff = mat.col;
      Vec col_refr = Vec();
      Vec col_trans = Vec();

      Vec hit = r.origin + r.dir * t;
      Vec n = (hit - spheres[id].pos).norm();
      Vec l = (light.pos - hit).norm();
      double lambert = clamp(n.dot(l));

      // reflection ray
      double refl_val = mat.refl;
      if (depth > 0 && refl_val > 0.0) {
         Vec refl_dir = r.dir - (n * (2 * r.dir.dot(n))); 
         Ray refl_ray = Ray(hit, refl_dir);
         col_refr = trace(refl_ray, depth - 1);
      }

      // refractive ray
      double refr_val = mat.trans;
      if (depth > 0 && refr_val > 0) {
         double test = 1 / n.dot(r.dir);
         Vec trans_dir = r.dir + n * test;
         trans_dir = trans_dir.norm();

    //     Vec c = (n % r.dir).norm();
    //     double theta = atan(c.y / c.x);
    //     double phi = acos(c.z);

         Ray trans_ray = Ray(hit, trans_dir);
         col_trans = trace(trans_ray, depth); 
      }

      // illumination
      Ray shadow = Ray(hit, l); 
      if (intersect(shadow, t, id)) {
         if (0 == id) { // if we hit light
            col_diff = col_diff * lambert;
         } else {
            col_diff = Vec();
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
         Vec d = cx*(((0 + .5 + 0)/2 + x)/WINDOW_WIDTH - .5) +
            cy*(((0 + .5 + 0)/2 + y)/WINDOW_HEIGHT - .5) + cam.dir;
         Ray r = Ray(cam.origin + d*140, d.norm());

         fill_pixel(glBuffer[y][x], trace(r, 5));
      }
   }

   /* refresh window buffer */
   glClear(GL_COLOR_BUFFER_BIT);
   glDrawPixels(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, glBuffer);
   SDL_GL_SwapWindow(window);
}
