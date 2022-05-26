#include <vecmath.h>
#include "extra.h"

Vector3f project (Vector4f v) {
  return Vector3f(v.x(), v.y(), v.z()); 
}

Vector4f expand (Vector3f v) {
  return Vector4f(v.x(), v.y(), v.z(), 1.f); 
}

Matrix4f homogenized (Matrix3f m) {
  Matrix4f m_; 
  m_(3, 3) = 1;
  for (int i = 0; i < 3; i++) 
    for (int j = 0; j < 3; j++) 
      m_(i, j) = m(i, j); 
  return m_;
}
