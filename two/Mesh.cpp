#include "Mesh.h"
#include "extra.h"
#include <fstream>

using namespace std;

const static int MAX_BUFFER_SIZE = 200; 

void Mesh::load( const char* filename )
{
  // 2.1.1. load() should populate bindVertices, currentVertices, and faces

  // Add your code here.

  // make a copy of the bind vertices as the current vertices
  ifstream in(filename); 
  char buffer[MAX_BUFFER_SIZE]; 
  while (in.getline(buffer, MAX_BUFFER_SIZE)) {
    stringstream ss(buffer); 
    string cmd; ss >> cmd; 
    if (cmd == "v") {
      Vector3f v; 
      ss >> v[0] >> v[1] >> v[2]; 
      bindVertices.push_back(v); 
    } else if (cmd == "f") {
      Tuple3u t; 
      ss >> t[0] >> t[1] >> t[2];
      t[0]--;
      t[1]--;
      t[2]--;
      faces.push_back(t); 
    }
  }
  currentVertices = bindVertices;
}

void Mesh::draw()
{
  // Since these meshes don't have normals
  // be sure to generate a normal per triangle.
  // Notice that since we have per-triangle normals
  // rather than the analytical normals from
  // assignment 1, the appearance is "faceted".
  glBegin(GL_TRIANGLES);
  for (auto &tup: faces) {
    int i = tup[0], j = tup[1], k = tup[2]; 
    Vector3f a = currentVertices[i], b = currentVertices[j], c = currentVertices[k]; 
    Vector3f normal = Vector3f::cross(b - a, c - b).normalized(); 
    glNormal(expand(normal)); 
    glVertex(expand(currentVertices[i])); 
    glNormal(expand(normal)); 
    glVertex(expand(currentVertices[j])); 
    glNormal(expand(normal)); 
    glVertex(expand(currentVertices[k])); 
  }
  glEnd(); 
}

void Mesh::loadAttachments( const char* filename, int numJoints )
{
  // 2.2. Implement this method to load the per-vertex attachment weights
  // this method should update m_mesh.attachments
  ifstream in(filename); 
  char buffer[MAX_BUFFER_SIZE]; 
  while (in.getline(buffer, MAX_BUFFER_SIZE)) {
    stringstream ss(buffer); 
    attachments.emplace_back(numJoints); 
    attachments.back()[0] = 0.f; 
    for (int i = 1; i < numJoints; i++)
      ss >> attachments.back()[i]; 
  }
}
