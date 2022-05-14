#include <GL/glut.h>
#include <iostream>
#include <vecmath.h>
#include <vector> 
#include "mesh.h"

using namespace std;

inline void glVertex(const Vector4f &a) { 
  float arr[3] = { a.x(), a.y(), a.z() }; 
  glVertex3fv(arr); 
}

inline void glNormal(const Vector4f &a) { 
  float arr[3] = { a.x(), a.y(), a.z() }; 
  glNormal3fv(arr); 
}

void Mesh::draw () {
  glBegin(GL_TRIANGLES);
  for (int i = 0; i < faces.size(); i++)
    if (invalidFaces.count(i) == 0) 
      for (int i : faces[i].cornerIds) {
        glNormal(vertices[i].vn); 
        glVertex(vertices[i].v); 
      }
  glEnd(); 
}
