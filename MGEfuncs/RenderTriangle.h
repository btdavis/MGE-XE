#ifndef RENDERTRIANGLE_H
#define RENDERTRIANGLE_H

class RoamTriangleNode;

#include "SplitTriangle.h"

class RenderTriangle {
public:
    SplitTriangle st;
    unsigned int left_index, right_index, top_index;
    RoamTriangleNode* rt;

    RenderTriangle(const SplitTriangle& s_tri, RoamTriangleNode* r_tri);
    ~RenderTriangle();
};

#endif
