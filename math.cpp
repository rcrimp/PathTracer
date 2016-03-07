/* stolen from smallpt.cpp
 * replace with a clean math class */

inline double clamp(double x) {
   return x < 0 ? 0 ? x > 1 : 1 : x;
}

inline int toInt(double x) {
   // return int(pow(clamp(x), 1/2.2)*255+.5);
   return int(clamp(x)*255);
}

/* Point, Vector, Color */
struct Vec {
   double x, y, z; // r, g, b

   Vec(double x_ = 0, double y_ = 0, double z_ = 0) {
      x = x_;
      y = y_;
      z = z_;
   }

   Vec operator+(const Vec &b) const {
      return Vec(x + b.x, y + b.y, z + b.z);
   } 

   Vec operator-(const Vec &b) const {
      return Vec(x - b.x, y - b.y, z - b.z);
   } 

   Vec operator*(double b) const {
      return Vec(x * b, y * b, z * b);
   } 

   Vec mult(const Vec &b) const {
      return Vec(x * b.x, y * b.y, z * b.z);
   }

   Vec& norm() {
      return *this = *this * (1/sqrt(x*x + y*y + z*z));
   }

   double dot(const Vec &b) const {
      return x*b.x + y*b.y + z*b.z;
   }

   Vec operator%(Vec &b) { // CROSS
      return Vec(y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x);
   }
};

struct Ray {
   Vec origin, dir;
   Ray(Vec o_, Vec d_) :
      origin(o_),
      dir(d_) {

      }
};

struct Sphere {
   double rad;
   Vec pos, col, emission;
   Sphere(double rad_, Vec p_, Vec e_, Vec c_) :
      rad(rad_),
      pos(p_),
      col(c_),
      emission(e_) {

      }
   double intersect(const Ray &r) const {
      Vec op = pos - r.origin;
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
