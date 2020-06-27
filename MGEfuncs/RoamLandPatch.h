#ifndef ROAMLANDPATCH_H
#define ROAMLANDPATCH_H

#include "RoamTriangleNode.h"
#include "HeightFieldSampler.h"
#include "LandMesh.h"
#include "nif_math.h"

class RoamLandPatch {
public:
    float top, left, bottom, right;
    RoamTriangleNode t1, t2;
    HeightFieldSampler* sampler;

    RoamLandPatch();
    RoamLandPatch(float t, float l, float b, float r, HeightFieldSampler* s);
    RoamLandPatch(const RoamLandPatch& patch);
    ~RoamLandPatch();

    Niflib::Vector3 GetTopLeft();
    Niflib::Vector3 GetBottomLeft();
    Niflib::Vector3 GetTopRight();
    Niflib::Vector3 GetBottomRight();

    void ConnectRightNeighbor(RoamLandPatch& neighbor);
    void ConnectLeftNeighbor(RoamLandPatch& neighbor);
    void ConnectTopNeighbor(RoamLandPatch& neighbor);
    void ConnectBottomNeighbor(RoamLandPatch& neighbor);

    void Tesselate(float max_variance, size_t tree_depth);

    LandMesh GenerateMesh(unsigned int cache_size);

    // void DebugPrint();
};

#endif
