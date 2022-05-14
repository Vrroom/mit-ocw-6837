#include "mesh.h" 
#include <string>
#include <sstream>
#include <iostream>

using namespace std;

const int MAX_BUFFER_SIZE = 200;

vector<unsigned> split (string s, string delim) {
  // helper function to split string using delimiter
  size_t pos = 0;
  string token;
  vector<unsigned> ids; 
  while ((pos = s.find(delim)) != string::npos) {
    token = s.substr(0, pos);
    s.erase(0, pos + delim.length());
    ids.push_back((unsigned) stoi(token)); 
  }
  ids.push_back((unsigned) stoi(s)); 
  return ids;
}

void Mesh::read () {
  // load the OBJ file here
  char buffer[MAX_BUFFER_SIZE]; 
  // Read input and store vertices, normals and
  // faces. Once the complete connectivity information
  // is available, populate the mesh data structure. 
  vector<Vector4f> vecv, vecn; 
  vector<vector<unsigned> > vecf; 
  while (cin.getline(buffer, MAX_BUFFER_SIZE)) {
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
        vector<unsigned> v = split(a, "/");
        auto &back = vecf.back(); 
        back.insert(back.end(), v.begin(), v.end()); 
      }
    }
  }
  vector<set<int> > fids(vecv.size()); // list of face ids for each vertex
  vector<int> vnid(vecv.size()); 
  for (int j = 0; j < vecf.size(); j++) {
    auto &ids = vecf[j]; 
    for (auto &num: ids) num--; 
    int a = ids[0], c = ids[2];
    int d = ids[3], f = ids[5];
    int g = ids[6], i = ids[8];
    faces.emplace_back(j, vector<int>({a, d, g})); 
    fids[a].insert(j); 
    fids[d].insert(j); 
    fids[g].insert(j); 
    vnid[a] = c;
    vnid[d] = f;
    vnid[g] = i;
  }
  for (int i = 0; i < vecv.size(); i++)
    vertices.emplace_back(i, vecv[i], vecn[vnid[i]], fids[i]); 
}
