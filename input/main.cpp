#include <stdlib.h>
#include <stdio.h>
#include <Dense>

using namespace Eigen;

struct Material {
   const char* name; // unique name
   Vector3d col; // base colour 
   double diff; // diffuse amount
   double refl; // reflection amount
   double trans; // tranmission amount

   Material(
         const char* name_ = "",
         Vector3d col_ = Vector3d(1.0, 1.0, 1.0),
         double diff_ = 1.0,
         double refl_ = 0.0,
         double trans_ = 0.0) : 
      name(name_),
      col(col_),
      diff(diff_),
      refl(refl_),
      trans(trans_)
   {}
};

const Material & mat_default = Material();

struct Camera {
   Vector3d origin;
   Vector3d dir;     

   Camera() {
      origin = Vector3d(0, 0, -3);
      dir = Vector3d(0, 0, 1);
   }
};

struct Sphere {
   Vector3d pos; // x y z position
   Material &mat; // material
   double rad; // radius

   Sphere() {
      pos = Vector3d(0.0, 0.0, 0.0); // origin
      mat = NULL;
      rad = 1.0;
   }

   double intersect(const Ray &r) const {
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
};

// TODO change static arrays to dynamic
struct Scene {
   Camera camera;
   Material materials[10];
   Sphere spheres[10];
   int material_count;
   int sphere_count;

   Scene() { // default scene setup
      camera = Camera(); 
      materials[0] = Material();
      spheres[0] = Sphere();
      spheres[0].mat = materials[0];
      material_count = 1;
      sphere_count = 1;
   }
};

bool load_scene(const char* filename, Scene &scene) {
   FILE *f;
   fprintf(stderr, "loading scene file: %s\n", filename);
   if (!(f = fopen(filename, "r"))) {
      fprintf(stderr, "unable to open file %s\n", filename);
   }

   char line[256];
   while (fgets(line, sizeof(line), f)) {
      if (line[0] == '#') continue;
      fprintf(stderr, "%s", line);
   }


}

int main(int argc, char* argv[]) {
   if (argc < 2) {
      fprintf(stderr, "no scene file given on command line\n"); 
      return EXIT_FAILURE;
   }

   char* filename = argv[1];
   Scene scene = Scene();
   load_scene(filename, scene);
}
