#include <vecmath.h>
#include "extra.h"

Vector3f project (Vector4f v) {
  return Vector3f(v.x(), v.y(), v.z()); 
}

Vector4f expand (Vector3f v) {
  return Vector4f(v.x(), v.y(), v.z(), 1.f); 
}

