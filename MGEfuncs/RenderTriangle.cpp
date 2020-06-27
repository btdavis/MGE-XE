#include "StdAfx.h"
#include "RenderTriangle.h"
#include "RoamTriangleNode.h"

RenderTriangle::RenderTriangle(const SplitTriangle& s_tri, RoamTriangleNode* r_tri) : st(s_tri), rt(r_tri) {
    left_index = 0xFFFFFFFF;
    right_index = 0xFFFFFFFF;
    top_index = 0xFFFFFFFF;
}

RenderTriangle::~RenderTriangle() {}
