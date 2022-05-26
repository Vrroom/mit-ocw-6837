#include "SkeletalModel.h"
#include "extra.h"
#include <fstream>

#include <FL/Fl.H>

using namespace std;

const static int MAX_BUFFER_SIZE = 200; 

void SkeletalModel::load(const char *skeletonFile, const char *meshFile, const char *attachmentsFile)
{
  loadSkeleton(skeletonFile);

  m_mesh.load(meshFile);
  m_mesh.loadAttachments(attachmentsFile, m_joints.size());

  computeBindWorldToJointTransforms();
  updateCurrentJointToWorldTransforms();
}

void SkeletalModel::draw(Matrix4f cameraMatrix, bool skeletonVisible)
{
  // draw() gets called whenever a redraw is required
  // (after an update() occurs, when the camera moves, the window is resized, etc)

  m_matrixStack.clear();
  m_matrixStack.push(cameraMatrix);
  if( skeletonVisible )
  {
    drawJoints();

    drawSkeleton();
  }
  else
  {
    // Clear out any weird matrix we may have been using for drawing the bones and revert to the camera matrix.
    glLoadMatrix(m_matrixStack.top());

    // Tell the mesh to draw itself.
    m_mesh.draw();
  }
}

void SkeletalModel::loadSkeleton( const char* filename )
{
  // Load the skeleton from file here.
  ifstream in(filename); 
  char buffer[MAX_BUFFER_SIZE]; 
  vector<Vector3f> ts; 
  vector<int> ps;
  if (!in) {
    cerr << "File not found! " << filename << endl;
    exit(0);
  }
  while(in.getline(buffer, MAX_BUFFER_SIZE)) {
    stringstream ss(buffer); 
    Vector3f v; 
    int p; 
    ss >> v[0] >> v[1] >> v[2] >> p; 
    ts.push_back(v); 
    ps.push_back(p); 
  }
  for (auto i : ps) {
    Joint * j = new Joint;
    m_joints.push_back(j); 
  }
  for (int i = 0; i < ps.size(); i++) {
    m_joints[i]->transform = Matrix4f::translation(ts[i]); 
    if (ps[i] == -1) 
      m_rootJoint = m_joints[i]; 
    else 
      m_joints[ps[i]]->children.push_back(m_joints[i]); 
  }
}

void SkeletalModel::_drawJoint (Joint * j) {
  m_matrixStack.push(j->transform);
  glLoadMatrix(m_matrixStack.top()); 
  glutSolidSphere(0.025f, 12, 12); 
  for (auto cj : j->children)
    _drawJoint(cj); 
  m_matrixStack.pop(); 
}

void SkeletalModel::drawJoints( )
{
  // Draw a sphere at each joint. You will need to add a recursive helper function to traverse the joint hierarchy.
  //
  // We recommend using glutSolidSphere( 0.025f, 12, 12 )
  // to draw a sphere of reasonable size.
  //
  // You are *not* permitted to use the OpenGL matrix stack commands
  // (glPushMatrix, glPopMatrix, glMultMatrix).
  // You should use your MatrixStack class
  // and use glLoadMatrix() before your drawing call.
  _drawJoint(m_rootJoint); 
}

void SkeletalModel::_drawSkeleton (Joint *j) {
  m_matrixStack.push(j->transform); 
  glLoadMatrix(m_matrixStack.top()); 
  for (auto cj : j->children) {
    Vector4f cp = cj->transform.getCol(3); 
    Vector3f cp3 = project(cp); 
    float len = cp3.abs();
    Vector3f z = cp3.normalized(); 
    Vector3f y = Vector3f::cross(z, Vector3f(0.f, 0.f, 1.0)).normalized(); 
    Vector3f x = Vector3f::cross(y, z).normalized(); 
    // push transformations.
    m_matrixStack.push(Matrix4f::translation(0.5 * cp3)); 
    m_matrixStack.push(homogenized(Matrix3f(x, y, z))); 
    m_matrixStack.push(Matrix4f::scaling(0.025f, 0.025f, len)); 
    // load and draw cube in current basis.
    glLoadMatrix(m_matrixStack.top()); 
    glutSolidCube(1.f); 
    // pop transformations.
    m_matrixStack.pop(); 
    m_matrixStack.pop();
    m_matrixStack.pop(); 
    _drawSkeleton(cj);  
  }
  m_matrixStack.pop(); 
}

void SkeletalModel::drawSkeleton( )
{
  // Draw boxes between the joints. You will need to add a recursive helper function to traverse the joint hierarchy.
  _drawSkeleton(m_rootJoint); 
}

void SkeletalModel::setJointTransform(int jointIndex, float rX, float rY, float rZ)
{
  // Set the rotation part of the joint's transformation matrix based on the passed in Euler angles.
  Matrix3f rot = Matrix3f::rotateX(rX) * Matrix3f::rotateY(rY) * Matrix3f::rotateX(rZ); 
  auto &mat = m_joints[jointIndex]->transform; 
  for (int i = 0; i < 3; i++) 
    for (int j = 0; j < 3; j++) 
      mat(i, j) = rot(i, j); 
}

void SkeletalModel::_computeBindWorldToJointTransforms (Joint *j, Matrix4f m) {
  m = m * j->transform; 
  j->bindWorldToJointTransform = m.inverse();
  for (auto cj : j->children)
    _computeBindWorldToJointTransforms(cj, m); 
}

void SkeletalModel::_updateCurrentJointToWorldTransforms (Joint *j, Matrix4f m) {
  m = m * j->transform;
  j->currentJointToWorldTransform = m;
  for (auto cj : j->children)
    _updateCurrentJointToWorldTransforms(cj, m); 
}

void SkeletalModel::computeBindWorldToJointTransforms()
{
  // 2.3.1. Implement this method to compute a per-joint transform from
  // world-space to joint space in the BIND POSE.
  //
  // Note that this needs to be computed only once since there is only
  // a single bind pose.
  //
  // This method should update each joint's bindWorldToJointTransform.
  // You will need to add a recursive helper function to traverse the joint hierarchy.
  _computeBindWorldToJointTransforms(m_rootJoint, Matrix4f::identity()); 
}

void SkeletalModel::updateCurrentJointToWorldTransforms()
{
  // 2.3.2. Implement this method to compute a per-joint transform from
  // joint space to world space in the CURRENT POSE.
  //
  // The current pose is defined by the rotations you've applied to the
  // joints and hence needs to be *updated* every time the joint angles change.
  //
  // This method should update each joint's bindWorldToJointTransform.
  // You will need to add a recursive helper function to traverse the joint hierarchy.
  _updateCurrentJointToWorldTransforms(m_rootJoint, Matrix4f::identity()); 
}

void SkeletalModel::updateMesh()
{
  // 2.3.2. This is the core of SSD.
  // Implement this method to update the vertices of the mesh
  // given the current state of the skeleton.
  // You will need both the bind pose world --> joint transforms.
  // and the current joint --> world transforms.
  for (int i = 0; i < m_mesh.currentVertices.size(); i++) {
    Vector4f v, p = expand(m_mesh.bindVertices[i]); 
    for (int j = 0; j < m_joints.size(); j++) {
      Matrix4f &binv = m_joints[j]->bindWorldToJointTransform;
      Matrix4f &t = m_joints[j]->currentJointToWorldTransform;
      v = v + (m_mesh.attachments[i][j] * (t * binv * p)); 
    }
    m_mesh.currentVertices[i] = project(v); 
  }
}

