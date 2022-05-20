#include "surf.h"
#include <vector>
#include "extra.h"
using namespace std;

namespace
{

  // We're only implenting swept surfaces where the profile curve is
  // flat on the xy-plane.  This is a check function.
  static bool checkFlat(const Curve &profile)
  {
    for (unsigned i=0; i<profile.size(); i++)
      if (profile[i].V[2] != 0.0 ||
          profile[i].T[2] != 0.0 ||
          profile[i].N[2] != 0.0)
        return false;

    return true;
  }
}

struct IdPair {
  int i, j, n, m; 
  IdPair (int i, int j, int n, int m) : i(i % n), j(j % m), n(n), m(m) {}
  int to1D () { return i * m + j; }
}; 

void makeFaces (int n, int m, Surface &surface, bool closed=false) {
  vector<vector<IdPair> > f; 
  for (int i = 0; i < n; i++) 
    for (int j = 0; j < m; j++) 
      if (closed || i + 1 < n) {
        f.push_back(vector<IdPair>({
          IdPair(i    , j    , n, m), 
          IdPair(i + 1, j    , n, m),
          IdPair(i    , j + 1, n, m)
        })); 
        f.push_back(vector<IdPair>({
          IdPair(i    , j + 1, n, m), 
          IdPair(i + 1, j    , n, m), 
          IdPair(i + 1, j + 1, n, m)
        })); 
      }
  for (auto &tup: f) 
    surface.VF.emplace_back(
      tup[0].to1D(), 
      tup[1].to1D(), 
      tup[2].to1D()
    ); 
}


Surface makeSurfRev(const Curve &profile, unsigned steps)
{
  Surface surface;

  if (!checkFlat(profile))
  {
    cerr << "surfRev profile curve must be flat on xy plane." << endl;
    exit(0);
  }
  Curve profile_ = profile;
  for (auto &p : profile_) 
    p.N.negate(); 
  int n = profile_.size(), m = steps;
  float theta = 2.f * M_PI / (m + 0.f); 
  vector<Curve> S; 
  for (int i = 0; i < m; i++) {
    S.emplace_back(); 
    for (auto p : profile_) 
      S.back().push_back(p); 
    Matrix3f roty = Matrix3f::rotateY(theta);
    for (auto &p : profile_) { 
      p.V = roty * p.V; 
      p.T = roty * p.T; 
      p.B = roty * p.B; 
      p.N = roty * p.N; 
      p.N.normalize();
    }
  }
 for (int i = 0; i < n; i++) 
   for (int j = 0; j < m; j++) {
     surface.VV.push_back(expand(S[j][i].V)); 
     surface.VN.push_back(expand(S[j][i].N)); 
   }
  makeFaces(n, m, surface); 
  return surface;
}

Surface makeGenCyl(const Curve &profile, const Curve &sweep )
{
  Surface surface;

  if (!checkFlat(profile))
  {
    cerr << "genCyl profile curve must be flat on xy plane." << endl;
    exit(0);
  }
  
  Curve profile_ = profile;
  for (auto &p : profile_) 
    p.N.negate(); 
  int n = profile_.size(), m = sweep.size();
  vector<vector<Vector3f> > VV, VN; 
  for (auto &sp : sweep) {
    VV.emplace_back(); 
    VN.emplace_back(); 
    Matrix3f B(sp.N, sp.B, sp.T); 
    for (auto &pp : profile_) { 
      VV.back().push_back(sp.V + B * pp.V); 
      VN.back().push_back((B * pp.N).normalized()); 
    }
  }
 for (int i = 0; i < n; i++) 
   for (int j = 0; j < m; j++) {
     surface.VV.push_back(expand(VV[j][i])); 
     surface.VN.push_back(expand(VN[j][i])); 
   }
  makeFaces(n, m, surface); 

  return surface;
}

void drawSurface(const Surface &surface, bool shaded)
{
  // Save current state of OpenGL
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  if (shaded)
  {
    // This will use the current material color and light
    // positions.  Just set these in drawScene();
    glEnable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // This tells openGL to *not* draw backwards-facing triangles.
    // This is more efficient, and in addition it will help you
    // make sure that your triangles are drawn in the right order.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
  }
  else
  {        
    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glColor4f(0.4f,0.4f,0.4f,1.f);
    glLineWidth(1);
  }

  glBegin(GL_TRIANGLES);
  for (unsigned i=0; i<surface.VF.size(); i++)
  {
    glNormal(surface.VN[surface.VF[i][0]]);
    glVertex(surface.VV[surface.VF[i][0]]);
    glNormal(surface.VN[surface.VF[i][1]]);
    glVertex(surface.VV[surface.VF[i][1]]);
    glNormal(surface.VN[surface.VF[i][2]]);
    glVertex(surface.VV[surface.VF[i][2]]);
  }
  glEnd();

  glPopAttrib();
}

void drawNormals(const Surface &surface, float len)
{
  // Save current state of OpenGL
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glDisable(GL_LIGHTING);
  glColor4f(0,1,1,1);
  glLineWidth(1);

  glBegin(GL_LINES);
  for (unsigned i=0; i<surface.VV.size(); i++)
  {
    glVertex(surface.VV[i]);
    glVertex(surface.VV[i] + surface.VN[i] * len);
  }
  glEnd();

  glPopAttrib();
}

void outputObjFile(ostream &out, const Surface &surface)
{

  for (unsigned i=0; i<surface.VV.size(); i++)
    out << "v  "
      << surface.VV[i][0] << " "
      << surface.VV[i][1] << " "
      << surface.VV[i][2] << endl;

  for (unsigned i=0; i<surface.VN.size(); i++)
    out << "vn "
      << surface.VN[i][0] << " "
      << surface.VN[i][1] << " "
      << surface.VN[i][2] << endl;

  out << "vt  0 0 0" << endl;

  for (unsigned i=0; i<surface.VF.size(); i++)
  {
    out << "f  ";
    for (unsigned j=0; j<3; j++)
    {
      unsigned a = surface.VF[i][j]+1;
      out << a << "/" << "1" << "/" << a << " ";
    }
    out << endl;
  }
}
