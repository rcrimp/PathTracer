#pragma once

#include <Dense>
#include <vector>
#include <string>

using namespace Eigen;

class Ray;
class Material;
class Sphere;
class Scene;
class Camera;

class Ray {
   public:
      Vector3d origin, dir;

      Ray();
      Ray(Vector3d, Vector3d);
};

class Material {
   public:
      char* name; // unique identifier for file parsing
      Vector3d col; // base colour
      double diff; // diffusion 
      double refl; // reflectance 
      double trans; // transmission
      double emit; // emission

      Material();
      Material(char*, Vector3d, double, double, double, double);
};

class Camera {
   public:
      Vector3d pos;
      Vector3d dir;
      double fov; // unused

      Camera();
      Camera(Vector3d, Vector3d, double);
};

class Sphere {
   public:
      Vector3d pos;
      double rad;
      Material mat;

      Sphere();
      Sphere(Vector3d, double, Material);
      double intersect(const Ray &) const;
};

//enum Object { none, material, sphere };

class Scene {
   public:
      Camera cam;
      std::vector<Material> materials;
      std::vector<Sphere> spheres;

      Scene();
      void print();
      bool load_scene(const char*);
   private:
      //Object last = none;
      Vector3d parse_vector(const char*);
      bool parse_line(const char *);
};

