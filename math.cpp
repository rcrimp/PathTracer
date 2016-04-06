#include "math.h"
#include <string.h>
#include <stdio.h>
#include <iostream>

using namespace Eigen;

Ray::Ray(){
   origin = Vector3d(0, 0, 0);
   dir = Vector3d(0, 0, 1);
}

Ray::Ray(Vector3d o_, Vector3d d_) :
   origin(o_),
   dir(d_) {

   }

Material::Material() {
   name = "default_mat";
   col = Vector3d(1.0, 1.0, 1.0);
   diff = 1.0;
   refl = 0.0;
   trans = 0.0;
   emit = 0.0;
}

Material::Material(char* name_, Vector3d col_, double diff_, double refl_, double trans_, double emit_) :
   name(name_), col(col_), diff(diff_), refl(refl_), trans(trans_), emit(emit_) {

   }

Sphere::Sphere(Vector3d p_, double rad_, Material mat_) :
   rad(rad_), pos(p_), mat(mat_) {

   }

Sphere::Sphere() {
   pos = Vector3d(0, 0, 0);
   rad = 1.0;
   mat = Material();
}

Camera::Camera() {
   pos = Vector3d(0, 0, -3);
   dir = Vector3d(0, 0, 1);
   fov = 45;
}

Camera::Camera(Vector3d pos_, Vector3d dir_, double fov_) :
   pos(pos_), dir(dir_), fov(fov_) {
   }

double Sphere::intersect(const Ray &r) const {
   Vector3d op = pos - r.origin;
   double t;
   double eps = 1.0e-4;
   double b = op.dot(r.dir);
   double det = b*b - op.dot(op) + rad*rad;
   if (det < 0) {
      return 0;
   } else {
      det = sqrt(det);
   }
   t = b - det;
   if (t > eps)
      return t;
   t = b + det;
   if (t > eps)
      return t;
   return 0;
}

Scene::Scene() {
   cam = Camera(Vector3d(50, 52, 295.6), Vector3d(0, 0, -1), 45);
   //   materials.push_back(Material("white", Vector3d(1, 1, 1), 1.0, 0.0, 0.0, 0.0));
   //   materials.push_back(Material("light", Vector3d(1, 1, 1), 0.0, 0.0, 0.0, 1.0));
   //   spheres.push_back(Sphere(Vector3d(50,81.6-16.5,81.6), 1.5, materials.front()));//Lite
   //   spheres.push_back(Sphere(Vector3d(57,16.5,37),        16.5,materials.front()));//Mirr
   //   spheres.push_back(Sphere(Vector3d(27,16.5,47),        16.5,materials.front())); //Mirr
   //spheres.push_back(Sphere(Vector3d(73,16.5,80),        16.5,materials.front())),//Glas
   //spheres.push_back(Sphere(Vector3d(1e5+1,40.8,81.6),   1e5, materials.front()));//Left
   //spheres.push_back(Sphere(Vector3d(-1e5+99,40.8,81.6), 1e5, materials.front()));//Rght
   //spheres.push_back(Sphere(Vector3d(50,40.8, 1e5),      1e5, materials.front()));//Back
   //spheres.push_back(Sphere(Vector3d(50,40.8,-1e5+170),  1e5, materials.front())); //Frnt
   //spheres.push_back(Sphere(Vector3d(50, 1e5, 81.6),     1e5, materials.front()));//Botm
   //spheres.push_back(Sphere(Vector3d(50,-1e5+81.6,81.6), 1e5, materials.front()));//Top
}

Vector3d Scene::parse_vector(const char* vec) {
   double x, y, z; 
   x = y = z = 0;
   if (3 != sscanf(vec, "%lg %lg %lg", &x, &y, &z))
      std::cerr << "error parsing vector: " << vec << std::endl;
   return Vector3d(x, y, z);
}

bool Scene::parse_line(const char* line) {
   // remove leading white space
   while (line[0] == ' ')
      line++;

   // ignore comments
   if (line[0] == '#')
      return true;

   // obtain the first key word
   char buff[128];
   if (1 != sscanf(line, "%s", buff))
      return false;
   const char* rest = line + strlen(buff);

   // parse the keyword  

   // cam_pos - camera position
   if (0 == strcmp(buff, "cam_pos")) {
      cam.pos = parse_vector(rest);
   }

   // cam_dir - camera direction
   else if (0 == strcmp(buff, "cam_dir")) {
      cam.dir = parse_vector(rest);
   }

   // new - new object/material 
   else if (0 == strcmp(buff, "new")) {
      if (1 == sscanf(rest, "%s", buff)) {
         if (0 == strcmp(buff, "material")){
            materials.push_back(Material()); 
         } else if (0 == strcmp(buff, "sphere")) {
            spheres.push_back(Sphere());
         }
      }
   }

   // material name
   else if (0 == strcmp(buff, "name")) {
      if (1 == sscanf(rest, "%s", buff)) {
         materials.back().name = strdup(buff); 
      }
   }

   // material color
   else if (0 == strcmp(buff, "color")) {
      materials.back().col = parse_vector(rest);
   }

   // diffuse, relfectance, transmission
   else if (0 == strcmp(buff, "scatter")) {
      double diff, refl, trans, emit; 
      if (4 != sscanf(rest, "%lg %lg %lg %lg", &diff, &refl, &trans, &emit))
         fprintf(stderr, "error parsing vector: %s\n", rest);

      materials.back().diff = diff;
      materials.back().refl = refl;
      materials.back().trans = trans;
      materials.back().emit = emit;
   }

   // sphere position
   else if (0 == strcmp(buff, "position")) {
      spheres.back().pos = parse_vector(rest);
   }

   // sphere radius
   else if (0 == strcmp(buff, "radius")) {
      double temp = 1.0;
      if (1 == sscanf(rest, "%lg", &temp)) {
         spheres.back().rad = temp;
      } 
   }

   // sphere material
   else if (0 == strcmp(buff, "material")) {
      if (1 == sscanf(rest, "%s", buff)) {
         for (Material &m : materials) {
            if (0 == strcmp(buff, m.name)){
               spheres.back().mat = m;
            }
         }      
      }
   }

   return true;
}

void Scene::print() {
   std::cout << "camera position:  " << cam.pos << std::endl;
   std::cout << "camera direction: " << cam.dir << std::endl;
   for (Sphere &s : spheres) {
      std::cout << " Sphere" << std::endl;
      std::cout << "pos:" << s.pos << std::endl;
      std::cout << "rad:" << s.rad << std::endl;
      std::cout << "mat:" << s.mat.name << std::endl;
   }
}

bool Scene::load_scene(const char *fn) {
   //TODO reset scene

   // open file for reading
   FILE *f;
   if (!(f = fopen(fn, "r"))) {
      fprintf(stderr, "unable to open file %s\n", fn);
      return false;
   }

   // parse each line
   char line[256];
   while (fgets(line, sizeof(line), f)) {
      parse_line(line);
   }

   return true;
}
