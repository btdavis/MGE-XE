#ifndef LANDMESH_H
#define LANDMESH_H

#include <vector>
#include "nif_math.h"
#include "LargeTriangle.h"

class LandMesh {
public:
    vector<Niflib::Vector3> vertices;
    vector<LargeTriangle> triangles;
    vector<Niflib::TexCoord> uvs;
    float radius;
    Niflib::Vector3 center;
    Niflib::Vector3 min;
    Niflib::Vector3 max;

    void CalcBounds(const Niflib::Vector3& new_min, const Niflib::Vector3& new_max);
    static bool SaveMeshes(LPCSTR file_path, vector<LandMesh>& meshes);
    bool Save(HANDLE& file);
};

#endif