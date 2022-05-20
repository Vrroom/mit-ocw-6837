#include "curve.h"
#include "extra.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
using namespace std;

namespace
{
  // Approximately equal to.  We don't want to use == because of
  // precision issues with floating point.
  inline bool approx( const Vector3f& lhs, const Vector3f& rhs )
  {
    const float eps = 1e-8f;
    return ( lhs - rhs ).absSquared() < eps;
  }


}

const Matrix4f BERNSTEIN(
    1.f, -3.f,  3.f, -1.f,
    0.f,  3.f, -6.f,  3.f,
    0.f,  0.f,  3.f, -3.f,
    0.f,  0.f,  0.f,  1.f
); 

const Matrix4f BSPLINE(
    1.f / 6.f, -3.f / 6.f,  3.f / 6.f, -1.f / 6.f,
    4.f / 6.f,  0.f / 6.f, -6.f / 6.f,  3.f / 6.f,
    1.f / 6.f,  3.f / 6.f,  3.f / 6.f, -3.f / 6.f,
    0.f / 6.f,  0.f / 6.f,  0.f / 6.f,  1.f / 6.f
); 

void evalBezierAtT (vector<Vector3f> &cp, float t, Vector3f &V, Vector3f &T) {
  Vector4f p (1.f,   t,   t * t,   t * t * t);
  Vector4f dp(0.f, 1.f, 2.f * t, 3.f * t * t);
  Vector4f xs(cp[0].x(), cp[1].x(), cp[2].x(), cp[3].x()); 
  Vector4f ys(cp[0].y(), cp[1].y(), cp[2].y(), cp[3].y()); 
  Vector4f zs(cp[0].z(), cp[1].z(), cp[2].z(), cp[3].z()); 
  V = Vector3f(
    Vector4f::dot(xs, BERNSTEIN * p), 
    Vector4f::dot(ys, BERNSTEIN * p), 
    Vector4f::dot(zs, BERNSTEIN * p)
  ); 
  T = Vector3f(
    Vector4f::dot(xs, BERNSTEIN * dp), 
    Vector4f::dot(ys, BERNSTEIN * dp), 
    Vector4f::dot(zs, BERNSTEIN * dp)
  ); 
  T.normalize();
}

void bsp2bez (const vector<Vector3f> &bsp, vector<Vector3f> &bez) {
  for (int i = 0; i + 3 < (int) bsp.size(); i++) {
    Vector4f gx(bsp[i][0], bsp[i + 1][0], bsp[i + 2][0], bsp[i + 3][0]); 
    Vector4f gy(bsp[i][1], bsp[i + 1][1], bsp[i + 2][1], bsp[i + 3][1]); 
    Vector4f gz(bsp[i][2], bsp[i + 1][2], bsp[i + 2][2], bsp[i + 3][2]); 
    auto M = (BSPLINE * BERNSTEIN.inverse()).transposed(); 
    Vector4f nx = M * gx; 
    Vector4f ny = M * gy; 
    Vector4f nz = M * gz; 
    if (i == 0) 
      bez.push_back(Vector3f(nx.x(), ny.x(), nz.x())); 
    for (int j = 1; j < 4; j++) 
      bez.push_back(Vector3f(nx[j], ny[j], nz[j])); 
  }
}

void rectifyNormal (Curve &c) {
  auto &P0 = c.front(); 
  auto &PN = c.back(); 
  if (approx(P0.V, PN.V) && approx(P0.T, PN.T) && !approx(P0.N, PN.N)) {
    Matrix3f B0(P0.N, P0.B, P0.T); 
    auto pn = B0.inverse() * PN.N; 
    float theta = -atan2(pn.y(), pn.x()); 
    float theta_ = theta; 
    int n = c.size(); 
    for (int i = n - 1; i >= 0; i--) {
      Matrix3f Bi(c[i].N, c[i].B, c[i].T); 
      c[i].N = Bi * Matrix3f::rotateZ(theta_) * Vector3f(1.f, 0.f, 0.f); 
      theta_ -= (theta / n); 
    }
    for (auto &pt: c) 
      pt.B = Vector3f::cross(pt.T, pt.N).normalized(); 
  }
}

Curve evalBezier( const vector< Vector3f >& P, unsigned steps )
{
  // Check
  if( P.size() < 4 || P.size() % 3 != 1 )
  {
    cerr << "evalBezier must be called with 3n+1 control points." << endl;
    exit( 0 );
  }

  // You should implement this function so that it returns a Curve
  // (e.g., a vector< CurvePoint >).  The variable "steps" tells you
  // the number of points to generate on each piece of the spline.
  // At least, that's how the sample solution is implemented and how
  // the SWP files are written.  But you are free to interpret this
  // variable however you want, so long as you can control the
  // "resolution" of the discretized spline curve with it.

  // Make sure that this function computes all the appropriate
  // Vector3fs for each CurvePoint: V,T,N,B.
  // [NBT] should be unit and orthogonal.

  // Also note that you may assume that all Bezier curves that you
  // receive have G1 continuity.  Otherwise, the TNB will not be
  // be defined at points where this does not hold.

  cerr << "\t>>> evalBezier has been called with the following input:" << endl;

  cerr << "\t>>> Control points (type vector< Vector3f >): "<< endl;
  for( unsigned i = 0; i < P.size(); ++i )
  {
    cerr << "\t>>> ";
    P[i].print();
    cerr << endl;
  }

  cerr << "\t>>> Steps (type steps): " << steps << endl;

  vector<Vector3f> V, T, N, B; 
  for (int i = 0; i + 3 < (int) P.size(); i += 3) {
    vector<Vector3f> cp; 
    for (int j = 0; j < 4; j++) 
      cp.push_back(P[i + j]); 
    for (int st = 0; st <= (int) steps; st++) {
      float t = (0.f + st) / (0.f + steps); 
      V.emplace_back(); 
      T.emplace_back(); 
      evalBezierAtT(cp, t, V.back(), T.back()); 
      if (B.empty())
        B.push_back(Vector3f(0.f, 0.f, 1.f)); 
      N.push_back(Vector3f::cross(B.back(), T.back()).normalized()); 
      if (!B.empty()) 
        B.push_back(Vector3f::cross(T.back(), N.back()).normalized()); 
    }
  }

  Curve c(V.size()); 
  for (int i = 0; i < (int) V.size(); i++) {
    c[i].V = V[i];
    c[i].T = T[i];
    c[i].N = N[i];
    c[i].B = B[i];
  }
  rectifyNormal (c); 
  return c;
}

Curve evalBspline( const vector< Vector3f >& P, unsigned steps )
{
  // Check
  if( P.size() < 4 )
  {
    cerr << "evalBspline must be called with 4 or more control points." << endl;
    exit( 0 );
  }

  // It is suggested that you implement this function by changing
  // basis from B-spline to Bezier.  That way, you can just call
  // your evalBezier function.

  cerr << "\t>>> evalBSpline has been called with the following input:" << endl;

  cerr << "\t>>> Control points (type vector< Vector3f >): "<< endl;
  for( unsigned i = 0; i < P.size(); ++i )
  {
    cerr << "\t>>> ";
    P[i].print();
    cerr << endl;
  }

  cerr << "\t>>> Steps (type steps): " << steps << endl;
  vector<Vector3f> bez; 
  bsp2bez(P, bez); 
  return evalBezier(bez, steps); 
}

Curve evalCircle( float radius, unsigned steps )
{
  // This is a sample function on how to properly initialize a Curve
  // (which is a vector< CurvePoint >).

  // Preallocate a curve with steps+1 CurvePoints
  Curve R( steps+1 );

  // Fill it in counterclockwise
  for( unsigned i = 0; i <= steps; ++i )
  {
    // step from 0 to 2pi
    float t = 2.0f * M_PI * float( i ) / steps;

    // Initialize position
    // We're pivoting counterclockwise around the y-axis
    R[i].V = radius * Vector3f( cos(t), sin(t), 0 );

    // Tangent vector is first derivative
    R[i].T = Vector3f( -sin(t), cos(t), 0 );

    // Normal vector is second derivative
    R[i].N = Vector3f( -cos(t), -sin(t), 0 );

    // Finally, binormal is facing up.
    R[i].B = Vector3f( 0, 0, 1 );
  }

  return R;
}

void drawCurve( const Curve& curve, float framesize )
{
  // Save current state of OpenGL
  glPushAttrib( GL_ALL_ATTRIB_BITS );

  // Setup for line drawing
  glDisable( GL_LIGHTING ); 
  glColor4f( 1, 1, 1, 1 );
  glLineWidth( 1 );

  // Draw curve
  glBegin( GL_LINE_STRIP );
  for( unsigned i = 0; i < curve.size(); ++i )
  {
    glVertex( expand(curve[ i ].V) );
  }
  glEnd();

  glLineWidth( 1 );

  // Draw coordinate frames if framesize nonzero
  if( framesize != 0.0f )
  {
    Matrix4f M;

    for( unsigned i = 0; i < curve.size(); ++i )
    {
      M.setCol( 0, Vector4f( curve[i].N, 0 ) );
      M.setCol( 1, Vector4f( curve[i].B, 0 ) );
      M.setCol( 2, Vector4f( curve[i].T, 0 ) );
      M.setCol( 3, Vector4f( curve[i].V, 1 ) );

      glPushMatrix();
      glMultMatrix( M );
      glScaled( framesize, framesize, framesize );
      glBegin( GL_LINES );
      glColor3f( 1, 0, 0 ); glVertex3d( 0, 0, 0 ); glVertex3d( 1, 0, 0 );
      glColor3f( 0, 1, 0 ); glVertex3d( 0, 0, 0 ); glVertex3d( 0, 1, 0 );
      glColor3f( 0, 0, 1 ); glVertex3d( 0, 0, 0 ); glVertex3d( 0, 0, 1 );
      glEnd();
      glPopMatrix();
    }
  }

  // Pop state
  glPopAttrib();
}

