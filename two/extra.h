#ifndef EXTRA_H
#define EXTRA_H

#include <GL/gl.h>
#include <vector> 
#include <vecmath.h>

#ifndef M_PI
#define M_PI  3.14159265358979
#endif

Vector3f project (Vector4f v); 

Vector4f expand (Vector3f v); 

Matrix4f homogenized (Matrix3f m); 

// Inline functions to help with drawing
inline void glVertex( const Vector4f a )
{
  float arr[3] = { a.x(), a.y(), a.z() }; 
  glVertex3fv(arr);
}

inline void glNormal( const Vector4f a ) 
{
  float arr[3] = { a.x(), a.y(), a.z() }; 
  glNormal3fv(arr);
}

inline void glLoadMatrix( const Matrix4f m )
{
  float matrix[16]; 
  int id = 0;
  for (int j = 0; j < 4; j++) 
    for (int i = 0; i < 4; i++) 
      matrix[id++] = m(i, j); 
  glLoadMatrixf( matrix );
}

inline void glMultMatrix( const Matrix4f m )
{
  float matrix[16];
  int id = 0;
  for (int j = 0; j < 4; j++) 
    for (int i = 0; i < 4; i++) 
      matrix[id++] = m(i, j); 
  glMultMatrixf( matrix );
}

#endif
