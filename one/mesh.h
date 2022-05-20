#ifndef MESH_H
#define MESH_H

#include <vecmath.h> 
#include <algorithm> 
#include <utility> 
#include <initializer_list> 
#include <set> 
#include <vector> 
#include <iostream>
#include <cstdlib>
#include <fstream>

using namespace std;

typedef set<int> SI; 
typedef initializer_list<int> Ini; 
typedef initializer_list<float> IniF; 
typedef pair<int, int> P; 

Vector3f project (Vector4f v); 

struct VertexGeometry {
  Vector4f v, vn;

  VertexGeometry(); 

  VertexGeometry(Vector4f v, Vector4f vn); 
};

VertexGeometry linearCombination(const vector<VertexGeometry> &geoms, vector<float> cs); 

struct Vertex {
  int id;
  VertexGeometry geom; 
  SI fs, es;

  Vertex(int id, VertexGeometry geom); 

  Vertex(int id, VertexGeometry geom, SI &fs); 

  Vertex(int id, VertexGeometry geom, SI &fs, SI &es); 

  bool hasFace (int f); 

  bool hasEdge (int e); 

  void print(); 
};

struct Face {
  int id; 
  int vs[3], es[3];

  Face (int id, Ini vvs);

  Face (int id, Ini vvs, Ini ees);

  bool hasVert (int v); 

  bool hasEdge (int e); 

  void print();
}; 

struct Edge {
  int id; 
  int vs[2], fs[2];

  Edge (int id, Ini vvs);

  Edge (int id, Ini vvs, Ini ffs);

  int v(int u); 

  bool hasVert(int u);

  bool hasFace(int f); 

  bool operator < (const Edge &t) const; 

  void print();
};

struct Mesh {
  // To render stuff on OpenGL, you need oriented faces
  // that is why people use the half-edge data-structure. 
  // Unfortunately, I don't have this luxury, I don't know
  // what the half-edge data-structure is. So I'm going to
  // improvise.
  vector<Vertex> vertices;
  vector<Face> faces;
  vector<Edge> edges; 

  void loopSubdivide (); 

  void draw(); 

  bool read(ifstream &in); 

  void _orient (Face &a, Face &b); 

  bool consistent (); 

  void closure (); 

}; 

#endif
