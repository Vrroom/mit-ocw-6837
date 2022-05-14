#include <utility>
#include <set>
#include <cassert>
#include <cmath> 
#include <iostream>
#include "mesh.h"
#include "simplify.h"

using namespace std;

Vector3f proj (const Vector4f &x) {
  return Vector3f(x.x(), x.y(), x.z()); 
}

Matrix4f outer_prod (const Vector4f& x, const Vector4f &y) {
  Matrix4f prod; 
  for (int i = 0; i < 4; i++) 
    for (int j = 0; j < 4; j++) 
      prod(i, j) = x[i] * y[j];
  return prod;
}

Matrix4f operator + (const Matrix4f& x, const Matrix4f& y) {
  Matrix4f sum; // zeroes
  for( int i = 0; i < 4; i++)
    for( int j = 0; j < 4; j++)
      sum(i, j) = x(i, j) + y(i, j); 
  return sum;
}

Candidate::Candidate(GarlandHeckbert *gh, int v1, int v2) : gh(gh), v1(v1), v2(v2) {
  auto Q1 = gh->Qs[v1];
  auto Q2 = gh->Qs[v2];
  auto Q = Q1 + Q2, Q_ = Q1 + Q2;
  auto vert1 = gh->mesh.vertices[v1].v;
  auto vert2 = gh->mesh.vertices[v2].v;
  Q.setRow(3, Vector4f(0, 0, 0, 1)); 
  if (abs(Q.determinant()) < 1e-2) {
    error = INF;
    auto tvert = proj(vert1 + vert2) / 2;
    v_ = Vector4f(tvert.x(), tvert.y(), tvert.z(), 1.0f); 
  } else {
    v_ = Q.inverse() * Vector4f(0, 0, 0, 1); 
    v_.homogenize(); 
    error = Vector4f::dot(v_, Q_ * v_); 
  }
}

bool Candidate::operator < (const Candidate &t) const { 
  if (error == t.error) 
    return make_pair(v1, v2) < make_pair(t.v1, t.v2);  
  return error < t.error;
}

GarlandHeckbert::GarlandHeckbert (Mesh &mesh) : mesh(mesh) {
  computeQuadrics(); 
  initializeCandidates(); 
}

Vector4f GarlandHeckbert::_faceNormal (int fi) {
  // compute the coefficients of the plane made by the fi'th face:
  //    ax + by + cz + d = 0
  // such that a^2 + b^2 + c^2 = 1
  auto face = mesh.faces[fi];
  int i = face.cornerIds[0], j = face.cornerIds[1], k = face.cornerIds[2];
  Vector4f vi = mesh.vertices[i].v, vj = mesh.vertices[j].v, vk = mesh.vertices[k].v;
  Vector3f normal = Vector3f::cross(proj(vi - vj), proj(vi - vk)); 
  normal.normalize(); 
  float d = -Vector3f::dot(proj(vi), normal); 
  return Vector4f(normal.x(), normal.y(), normal.z(), d); 
}

void GarlandHeckbert::computeQuadrics () {
  int n = mesh.vertices.size(); 
  for (int i = 0; i < n; i++) {
    Qs.emplace_back();
    for (int fi: mesh.vertices[i].faceIds) {
      Vector4f fnormal = _faceNormal(fi); 
      Qs.back() = Qs.back() + outer_prod(fnormal, fnormal);  
    }
  }
}

void GarlandHeckbert::initializeCandidates () {
  set<pair<int, int> > pairs; 
  for (auto &f: mesh.faces) {
    int csz = f.cornerIds.size();
    for (int i = 0; i < csz; i++) { 
      int v1 = f.cornerIds[i], v2 = f.cornerIds[(1 + i) % csz]; 
      if (pairs.count(make_pair(v1, v2)) == 0) {
        candidates.emplace(this, v1, v2); 
        // insert both to avoid double counting
        pairs.emplace(v1, v2); 
        pairs.emplace(v2, v1); 
      }
    } 
  }
}

void GarlandHeckbert::simplifyStep () {
  auto &vertices = mesh.vertices;
  auto &faces = mesh.faces;
  while (!candidates.empty()) {
    auto tp = *candidates.begin(); 
    candidates.erase(candidates.begin()); 
    int v1 = tp.v1, v2 = tp.v2;
    // if this candidate has already been contracted then skip it. 
    if (mesh.invalidVertices.count(v1) > 0 ||
        mesh.invalidVertices.count(v2) > 0) 
      continue;
    // record faces shared by this candidate
    vector<int> fids;
    // old vertex faces need to be updated.
    for (auto fi : vertices[v1].faceIds) 
      if (mesh.invalidFaces.count(fi) == 0)
        fids.push_back(fi); 
    for (auto fi : vertices[v2].faceIds) 
      if (mesh.invalidFaces.count(fi) == 0)
        fids.push_back(fi); 
    // remove these faces from the mesh 
    for (auto fi : fids)
      mesh.invalidFaces.insert(fi); 
    // new vertex id is going to be the number of vertices
    // currently in the mesh.
    int vnew = vertices.size(); 
    // estimate the new vertex' normal by averaging the normals
    // of the old vertices. 
    auto vn = (vertices[v1].vn + vertices[v2].vn) / 2;
    auto vn_ = proj(vn).normalized();
    vn = Vector4f(vn_.x(), vn_.y(), vn_.z(), 1.0f); 
    // make new faces and record new edges.
    set<int> newfaces; 
    vector<pair<int, int> > newedges;
    for (auto fi : fids) { 
      vector<int> notInCandidate;
      for (auto vi : faces[fi].cornerIds)
        if (vi != v1 && vi != v2) {
          notInCandidate.push_back(vi); 
        }
      if (notInCandidate.size() == 2) {
        int fnew = faces.size(); 
        for (auto vnc : notInCandidate) {
          newedges.emplace_back(vnc, vnew); 
          vertices[vnc].faceIds.insert(fnew); 
        }
        notInCandidate.push_back(vnew); 
        newfaces.insert(fnew); 
        faces.emplace_back(fnew, notInCandidate); 
      }
    }
    // insert new vertex.
    vertices.emplace_back(vnew, tp.v_, vn, newfaces); 
    // Find the new q matrix for the new vertex. 
    Qs.emplace_back(); 
    Qs.back() = Qs[v1] + Qs[v2]; 
    // mark the handled vertices as invalid for future use.
    mesh.invalidVertices.insert(v1); 
    mesh.invalidVertices.insert(v2); 
    // now insert new candidates 
    for (auto &p: newedges) 
      candidates.emplace(this, p.first, p.second); 
    break; 
  }
}
