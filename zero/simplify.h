#include <vecmath.h> 
#include <algorithm>
#include <vector>
#include <cfloat>

using namespace std;

const double INF = DBL_MAX;

struct GarlandHeckbert;

struct Candidate {
  GarlandHeckbert * gh;
  int v1, v2; 
  double error; 
  Vector4f v_; 
  Candidate (GarlandHeckbert *gh, int v1, int v2); 
  bool operator < (const Candidate &t) const; 
};

struct GarlandHeckbert {
  Mesh &mesh;
  vector<Matrix4f> Qs;
  set<Candidate> candidates;   

  GarlandHeckbert (Mesh &mesh);

  void _computeQuadrics(); 

  Vector4f _faceNormal(int i); 

  void _initializeCandidates(); 
  
  void simplifyStep (); 
}; 

