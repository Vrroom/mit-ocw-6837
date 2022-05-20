#include "mesh.h"
#include "extra.h"
#include <sstream>
#include <map> 

using namespace std;

const int MAX_BUFFER_SIZE = 200;

VertexGeometry::VertexGeometry() {}

VertexGeometry::VertexGeometry (Vector4f v, Vector4f vn) : v(v), vn(vn) {
  vn = expand(project(vn).normalized()); 
}

VertexGeometry linearCombination (const vector<VertexGeometry> &geoms, vector<float> cs) {
  VertexGeometry g; 
  for (int i = 0; i < (int) geoms.size(); i++) {
    g.v  = g.v  + cs[i] * geoms[i].v ;
    g.vn = g.vn + cs[i] * geoms[i].vn;
  }
  g.vn = expand(project(g.vn).normalized()); 
  return g; 
}

Vertex::Vertex (int id, VertexGeometry geom) : id(id), geom(geom) {} 

Vertex::Vertex (int id, VertexGeometry geom, SI &fs) : id(id), geom(geom), fs(fs) {}; 

Vertex::Vertex (int id, VertexGeometry geom, SI &fs, SI &es) : id(id), geom(geom), fs(fs), es(es) {}; 

bool Vertex::hasFace (int f) { return fs.count(f) > 0; }

bool Vertex::hasEdge (int e) { return es.count(e) > 0; } 

void Vertex::print() {
  cout << "vertex id : " << id << endl;
  cout << "geometry : ";
  geom.v.print();
  geom.vn.print(); 
  cout << endl;
  cout << "faces : " << endl;
  for (int i : fs) 
    cout << i << " ";
  cout << endl;
  cout << "edges : " << endl;
  for (int i : es)
    cout << i << " ";
  cout << endl;
}

Face::Face (int id, Ini vvs) : id(id) {
  copy_n(vvs.begin(), 3, vs); 
}

Face::Face (int id, Ini vvs, Ini ees) : id(id) {
  copy_n(vvs.begin(), 3, vs); 
  copy_n(ees.begin(), 3, es); 
}

bool Face::hasVert (int v) {
  for (int i = 0; i < 3; i++)
    if (vs[i] == v) 
      return true;
  return false;
}

bool Face::hasEdge (int e) {
  for (int i = 0; i < 3; i++) 
    if (es[i] == e) 
      return true;
  return false;
} 

void Face::print() {
  cout << "face id : " << id << endl;
  cout << "vertices : " << endl;
  for (int i = 0; i < 3; i++)
    cout << vs[i] << " ";
  cout << endl;
  cout << "edges : " << endl;
  for (int i = 0; i < 3; i++)
    cout << es[i] << " ";
  cout << endl;
}

Edge::Edge (int id, Ini vvs) : id(id) {
  copy_n(vvs.begin(), 2, vs); 
}

Edge::Edge (int id, Ini vvs, Ini ffs) : id(id) {
  copy_n(vvs.begin(), 2, vs); 
  copy_n(ffs.begin(), 2, fs); 
}

int Edge::v(int u) { 
  return ((u == vs[0]) ? vs[1] : vs[0]);
}

bool Edge::hasVert(int u) { 
  return (u == vs[0] || u == vs[1]); 
}

bool Edge::hasFace (int f) {
  return (f == fs[0] || f == fs[1]); 
}

bool Edge::operator < (const Edge &t) const {
  auto p1 = make_pair(min(vs[0], vs[1]), max(vs[0], vs[1]));
  auto p2 = make_pair(min(t.vs[0], t.vs[1]), max(t.vs[0], t.vs[1]));
  return p1 < p2;
}

void Edge::print() {
  cout << "edge id : " << id << endl;
  cout << "vertices : " << endl;
  for (int i = 0; i < 2; i++)
    cout << vs[i] << " ";
  cout << endl;
  cout << "faces : " << endl;
  for (int i = 0; i < 2; i++)
    cout << fs[i] << " ";
  cout << endl;
}

void Mesh::draw () {
  // Save current state of OpenGL
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  // This will use the current material color and light
  // positions.  Just set these in drawScene();
  glEnable(GL_LIGHTING);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // This tells openGL to *not* draw backwards-facing triangles.
  // This is more efficient, and in addition it will help you
  // make sure that your triangles are drawn in the right order.
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glBegin(GL_TRIANGLES);
  for (auto &f: faces) {
    for (int i = 0; i < 3; i++) {
      glNormal(vertices[f.vs[i]].geom.vn); 
      glVertex(vertices[f.vs[i]].geom.v); 
    }
  }
  glEnd();

  glPopAttrib();
}

struct IdPair {
  int i, j;
  IdPair (int i, int j) : i(i), j(j) {}
};

IdPair _split (string s) {
  // helper function to split string using delimiter
  size_t pos = 0;
  string token;
  vector<unsigned> ids; 
  string delim = "/"; 
  while ((pos = s.find(delim)) != string::npos) {
    token = s.substr(0, pos);
    s.erase(0, pos + delim.length());
    if (!token.empty())
      ids.push_back((unsigned) stoi(token)); 
  }
  if (!s.empty()) 
    ids.push_back((unsigned) stoi(s)); 
  return IdPair(*ids.begin(), *ids.rbegin());
}

bool Mesh::read (ifstream &in) {
  // load the OBJ file here
  char buffer[MAX_BUFFER_SIZE]; 
  // Read input and store vertices, normals and faces. Once the 
  // complete connectivity information is available, populate the
  // mesh data structure. 
  vector<Vector4f> vecv, vecn; 
  vector<vector<IdPair> > vecf; 
  while (in.getline(buffer, MAX_BUFFER_SIZE)) {
    stringstream ss(buffer); 
    string cmd; ss >> cmd; 
    if (cmd == "v") {
      Vector4f v(0, 0, 0, 1); 
      ss >> v[0] >> v[1] >> v[2]; 
      vecv.push_back(v); 
    } else if (cmd == "vn") {
      Vector4f v(0, 0, 0, 1); 
      ss >> v[0] >> v[1] >> v[2]; 
      vecn.push_back(v); 
    } else if (cmd == "f") {
      vecf.emplace_back(); 
      while (ss.good()) {
        string a; ss >> a;
        vecf.back().push_back(_split(a)); 
        vecf.back().back().i--;
        vecf.back().back().j--;
      }
    }
  }
  vector<int> vert2norm(vecv.size()); 
  for (int j = 0; j < (int) vecf.size(); j++) {
    auto &ids = vecf[j]; 
    int a = ids[0].i, c = ids[0].j;
    int d = ids[1].i, f = ids[1].j;
    int g = ids[2].i, i = ids[2].j;
    faces.push_back(Face(j, {a, d, g})); 
    vert2norm[a] = c;
    vert2norm[d] = f;
    vert2norm[g] = i;
  }
  for (int i = 0; i < (int) vecv.size(); i++)
    vertices.emplace_back(i, VertexGeometry(vecv[i], vecn[vert2norm[i]])); 
  // take closure to make mesh consistent
  closure();
  if (!consistent()) {
    cerr << "* read obj incorrectly!! *" << endl;
    return false;
  }
  return true;
}

void Mesh::_orient (Face &a, Face &b) {
  // orient the face `a` with face `b`
  Vector3f n1, n2; 
  {
    Vector3f x = project(vertices[a.vs[0]].geom.v).normalized();
    Vector3f y = project(vertices[a.vs[1]].geom.v).normalized();
    Vector3f z = project(vertices[a.vs[2]].geom.v).normalized();
    n1 = Vector3f::cross(x - y, x - z); 
  }
  {
    Vector3f x = project(vertices[b.vs[0]].geom.v).normalized();
    Vector3f y = project(vertices[b.vs[1]].geom.v).normalized();
    Vector3f z = project(vertices[b.vs[2]].geom.v).normalized();
    n2 = Vector3f::cross(x - y, x - z); 
  }
  if (Vector3f::dot(n1, n2) < 0) 
    swap(a.vs[0], a.vs[1]); 
}

bool Mesh::consistent () {
  // check for consistency in ids.
  for (int i = 0; i < (int) vertices.size(); i++) 
    if (i != vertices[i].id) {
      cerr << "* vertex id mismatch - " << i << " " << vertices[i].id << " *" << endl;
      return false;
    }

  for (int i = 0; i < (int) faces.size(); i++) 
    if (i != faces[i].id) {
      cerr << "* face id mismatch - " << i << " " << faces[i].id << " *" << endl;
      return false;
    }

  for (int i = 0; i < (int) edges.size(); i++) 
    if (i != edges[i].id) {
      cerr << "* edges id mismatch - " << i << " " << edges[i].id << " *" << endl;
      return false;
    }

  // check whether each simplex contains ids of adjacent elements.
  for (int i = 0; i < (int) vertices.size(); i++) {
    for (auto j : vertices[i].es) 
      if (!edges[j].hasVert(i)) {
        cerr << "* vertex edge mismatch *" << endl;
        return false;
      }
    for (auto j : vertices[i].fs) 
      if (!faces[j].hasVert(i)) {
        cerr << "* vertex face mismatch *" << endl;
        return false;
      }
  }

  for (int i = 0; i < (int) faces.size(); i++) {
    for (int j = 0; j < 3; j++) 
      if (!edges[faces[i].es[j]].hasFace(i)) {
        cerr << "* face edge mismatch *" << endl;
        return false;
      }
    for (int j = 0; j < 3; j++) 
      if (!vertices[faces[i].vs[j]].hasFace(i)) {
        cerr << "* face vertex mismatch *" << endl;
        return false;
      }
  }

  for (int i = 0; i < (int) edges.size(); i++) {
    for (int j = 0; j < 2; j++) 
      if (!faces[edges[i].fs[j]].hasEdge(i)) {
        cerr << "* edge face mismatch *" << endl;
        return false;
      }
    for (int j = 0; j < 2; j++) 
      if (!vertices[edges[i].vs[j]].hasEdge(i)) {
        cerr << "* edge vertex mismatch *" << endl;
        return false;
      }
  }

  return true;

}

void Mesh::closure () {
  // Assume at this point that the vertices are ordered 
  // with the correct ids and the faces have the correct
  // vertex ids. All other information is dubious. 

  // Remove potentially false edge/face information in vertices.
  for (auto &v : vertices) {
    v.es.clear();
    v.fs.clear(); 
  }

  // Find the edges and record correspondence with faces. 
  map<Edge, vector<int> > edge2face; 
  for (int i = 0; i < (int) faces.size(); i++) {
    auto &f = faces[i]; 
    for (int j = 0; j < 3; j++) {
      int a = f.vs[j], b = f.vs[(1 + j) % 3];
      // init edge with dummy id for now. 
      edge2face[Edge(-1, {a, b})].push_back(i); 
    }
  }
  edges.clear(); 
  int eid = 0;
  for (auto p : edge2face) {
    edges.push_back(p.first); 
    // set correct id. 
    edges.back().id = eid++;
    for (int j = 0; j < 2; j++) 
      edges.back().fs[j] = p.second[j]; 
  }

  // At this point only edges have complete information.
  // Propagate edge and face ids to the vertices. 
  for (auto &f : faces)
    for (int i = 0; i < 3; i++) 
      vertices[f.vs[i]].fs.insert(f.id); 

  for (auto &e : edges)
    for (int i = 0; i < 2; i++) 
      vertices[e.vs[i]].es.insert(e.id); 

  // Now the faces need to know about the edges. 
  map<int, vector<int> > face2edge; 
  for (int i = 0; i < (int) edges.size(); i++) 
    for (int j = 0; j < 2; j++) 
      face2edge[edges[i].fs[j]].push_back(i); 
  for (int i = 0; i < (int) faces.size(); i++) 
    for (int j = 0; j < 3; j++) 
      faces[i].es[j] = face2edge[i][j];

  // At this point, mesh must be consistent. If not
  // raise an error and exit. 
  if (!consistent()) {
    cerr << "* Mesh isn't consistent!! *" << endl;
    exit(0); 
  }
}

void Mesh::loopSubdivide () {

  int nv = vertices.size(), ne = edges.size(); 
  vector<VertexGeometry> even, odd;

  // compute new positions for even vertices
  for (auto &u: vertices) {
    vector<VertexGeometry> geoms; 
    for (auto ei : u.es) {
      int v = edges[ei].v(u.id); 
      geoms.push_back(vertices[v].geom); 
    }
    int valence = geoms.size(); 
    float BETA = 3.f / ((valence > 3) ? 8.f * valence : 16.f);
    vector<float> cs(valence, BETA); 
    geoms.push_back(u.geom); 
    cs.push_back(1.f - (BETA * valence)); 
    even.push_back(linearCombination(geoms, cs)); 
  }

  // compute new positions for even vertices
  for (auto &e: edges) { 
    vector<VertexGeometry> geoms;
    for (int i = 0; i < 2; i++) 
      for (int j = 0; j < 3; j++) {
        int fv = faces[e.fs[i]].vs[j];
        if (!e.hasVert(fv))
          geoms.push_back(vertices[fv].geom);
      }
    geoms.push_back(vertices[e.vs[0]].geom);
    geoms.push_back(vertices[e.vs[1]].geom);
    vector<float> cs = { 1.f / 8.f, 1.f / 8.f, 3.f / 8.f, 3.f / 8.f };
    odd.push_back(linearCombination(geoms, cs));  
  }
  
  // push the new vertices 
  for (auto &e: edges) 
    vertices.push_back(
      Vertex(
        nv + e.id, 
        linearCombination(
          vector<VertexGeometry>({ 
            vertices[e.vs[0]].geom,
            vertices[e.vs[1]].geom
          }),
          vector<float>({ 0.5f, 0.5f })
        )
      )
    );

  // compute new faces 
  vector<Face> newFaces; 
  for (auto &f : faces) {
    // the faces containing one `even` vertex
    // and 2 `odd` vertices. 
    for (int i = 0; i < 3; i++) {
      int vi = f.vs[i]; 
      int ev[2], id = 0;
      for (int j = 0; j < 3; j++) 
        if (edges[f.es[j]].hasVert(vi))
          ev[id++] = nv + f.es[j];
      newFaces.push_back(Face(
        newFaces.size(), 
        { vi, ev[0], ev[1] }
      )); 
      _orient(newFaces.back(), f); 
    }
    // the face containing 3 `odd` vertices
    newFaces.push_back(Face(
      newFaces.size(), 
      { nv + f.es[0], nv + f.es[1], nv + f.es[2] }
    )); 
    _orient(newFaces.back(), f); 
  }
  faces = newFaces; 

  // update connectivity information
  closure(); 

  // update the positions for all vertices
  for (int i = 0; i < nv; i++)
    vertices[i].geom = even[i];
  for (int i = 0; i < ne; i++) 
    vertices[nv + i].geom = odd[i]; 
}; 
