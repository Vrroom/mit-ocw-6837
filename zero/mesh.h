#include <vecmath.h> 
#include <set> 
#include <vector> 

using namespace std;

struct Vertex {
  int id;
  Vector4f v, vn;
  set<int> faceIds;

  Vertex (int id, Vector4f &v, Vector4f &vn, set<int> &faceIds) 
    : id(id), v(v), vn(vn), faceIds(faceIds) {}
};

struct Face {
  int id; 
  vector<int> cornerIds;
  Face (int id, vector<int> cornerIds) : id(id), cornerIds(cornerIds) {} 
}; 

struct Mesh {
  vector<Vertex> vertices;
  vector<Face> faces;
  set<int> invalidFaces, invalidVertices;

  void computeQuadrics(); 

  void read(); // read mesh from stdin

  void draw(); // draw mesh in opengl

}; 

