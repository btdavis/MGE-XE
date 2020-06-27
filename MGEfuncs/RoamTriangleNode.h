#ifndef ROAMTRIANGLENODE_H
#define ROAMTRIANGLENODE_H

#include "HeightFieldSampler.h"
#include "SplitTriangle.h"
#include "RoamVarianceNode.h"
#include "RenderTriangle.h"

class RoamTriangleNode {
public:
    RoamTriangleNode* left_child;
    RoamTriangleNode* right_child;
    RoamTriangleNode* base_neighbor;
    RoamTriangleNode* left_neighbor;
    RoamTriangleNode* right_neighbor;

    RoamTriangleNode();
    ~RoamTriangleNode();

    void Split();

private:
    void EdgeSplit();
    void RelinkNeighbor(RoamTriangleNode* neighbor, RoamTriangleNode* old_link, RoamTriangleNode* new_link);
    void DiamondSplit();

public:
    void Tesselate(RoamVarianceNode* v_tri, float max_variance);
    void GatherTriangles(HeightFieldSampler* sampler, const SplitTriangle& s_tri, vector<RenderTriangle>& triangles);
    // void DebugPrint( size_t depth = 0);
};

#endif
