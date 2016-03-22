/* stolen from smallpt.cpp
 * replace with a clean math class */
#include <Dense>

using namespace Eigen;

inline double clamp(double x) {
   return x < 0 ? 0 ? x > 1 : 1 : x;
}

inline int toInt(double x) {
   // return int(pow(clamp(x), 1/2.2)*255+.5);
   return int(clamp(x)*255);
}

struct Ray {
   Vector3d origin, dir;

   Ray(){
      origin = Vector3d(0, 0, 0);
      dir = Vector3d(0, 0, 1);
   }

   Ray(Vector3d o_, Vector3d d_) :
      origin(o_),
      dir(d_) {

      }
};

struct Material {
   Vector3d col, emi;
   double diff, refl, trans;

   Material(Vector3d col_, Vector3d emi_, double diff_, double refl_, double trans_) :
      col(col_), emi(emi_), diff(diff_), refl(refl_), trans(trans_) {
   
   }
};

struct Sphere {
   double rad;
   Vector3d pos;
   Material mat;
   Sphere(double rad_, Vector3d p_, Material mat_) :
      rad(rad_), pos(p_), mat(mat_) {

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
