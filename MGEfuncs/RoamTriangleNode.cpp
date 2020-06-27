#include "StdAfx.h"
#include "RoamTriangleNode.h"
#include "RoamVarianceNode.h"
#include "HeightFieldSampler.h"

using namespace Niflib;

RoamTriangleNode::RoamTriangleNode() : left_child(0), right_child(0), base_neighbor(0), left_neighbor(0), right_neighbor(0) {}

RoamTriangleNode::~RoamTriangleNode() {
    if (left_child) {
        delete left_child;
    }
    if (right_child) {
        delete right_child;
    }
}

void RoamTriangleNode::Split() {
    // Check whether this node has already been split
    if (left_child || right_child) {
        // This node has already been split
        return;
    }

    // Check if the hypotonuse of this triangle is on an edge (has no base neighbor)
    if (!base_neighbor) {
        // This is on an edge, so split this triangle only
        EdgeSplit();
        return;
    }

    // Check if this triangle and its base neighbor form a diamond (they are eachother's base neighbor)
    if (base_neighbor->base_neighbor == this) {
        // split this triangle and its neighbor
        DiamondSplit();
        return;
    }

    // These triangles don't form a diamond, so call split on the base neighbor before splitting this triangle
    base_neighbor->Split();

    // Split the triangle and its neighbor
    DiamondSplit();
}

void RoamTriangleNode::EdgeSplit() {
    assert(left_child == 0 && right_child == 0);

    // Create children
    left_child = new RoamTriangleNode();
    right_child = new RoamTriangleNode();

    // Set neighbor linkage
    left_child->base_neighbor = left_neighbor;
    left_child->left_neighbor = right_child;
    left_child->right_neighbor = 0;

    RelinkNeighbor(left_child->left_neighbor, this, left_child);
    RelinkNeighbor(left_child->base_neighbor, this, left_child);

    right_child->base_neighbor = right_neighbor;
    right_child->left_neighbor = 0;
    right_child->right_neighbor = left_child;

    RelinkNeighbor(right_child->left_neighbor, this, right_child);
    RelinkNeighbor(right_child->base_neighbor, this, right_child);

    // Clear neighbors of this object since it now just represents a node, not a real triangle
    // Don't clear base_neighbor since it should either already be null or cleared by the DiamondSplit function
    right_neighbor = 0;
    left_neighbor = 0;
}

void RoamTriangleNode::RelinkNeighbor(RoamTriangleNode* neighbor, RoamTriangleNode* old_link, RoamTriangleNode* new_link) {
    if (neighbor) {
        if (neighbor->base_neighbor == old_link) {
            neighbor->base_neighbor = new_link;
        }
        else if (neighbor->left_neighbor == old_link) {
            neighbor->left_neighbor = new_link;
        }
        else if (neighbor->right_neighbor == old_link) {
            neighbor->right_neighbor = new_link;
        }
    }
}

void RoamTriangleNode::DiamondSplit() {
    assert(base_neighbor != 0 && base_neighbor->base_neighbor == this);

    // Start with an edge split for both triangles
    EdgeSplit();
    base_neighbor->EdgeSplit();

    // Now set left and right neighbor links which are set to zero by the basic edge split
    right_child->left_neighbor = base_neighbor->left_child;
    left_child->right_neighbor = base_neighbor->right_child;

    base_neighbor->right_child->left_neighbor = left_child;
    base_neighbor->left_child->right_neighbor = right_child;

    // Clear base_neighbors of these objects since they now just represents a node, not a real triangle
    base_neighbor->base_neighbor = 0;
    base_neighbor = 0;

}

void RoamTriangleNode::Tesselate(RoamVarianceNode* v_tri, float max_variance) {
    // Determine if the variance of this triangle is enough that we want to split it
    if (v_tri->variance < max_variance) {
        // The variance is within tolerable levels, so end the recursion of this branch
        return;
    }

    // The variance is too high, so we need to split this triangle
    Split();

    // If the variance triangle has children, continue on down the tree recursivly
    if (!v_tri->left_child || !v_tri->right_child) {
        // The variance triangle has no children, so we're done
        return;
    }

    // The variance triangle has children so call this function on the newly created children of this roam triangle node
    left_child->Tesselate(v_tri->left_child, max_variance);
    right_child->Tesselate(v_tri->right_child, max_variance);
}

void RoamTriangleNode::GatherTriangles(HeightFieldSampler* sampler, const SplitTriangle& s_tri, vector<RenderTriangle>& triangles) {

    Vector3 hc = s_tri.GetHypoCenter();
    hc.z = sampler->SampleHeight(hc.x, hc.y);

    if (!left_child || !right_child) {
        // This node has no children, so add the triangle to the list and return
        triangles.push_back(RenderTriangle(s_tri, this));
        return;
    }

    // This node has children, so call this function on them
    left_child->GatherTriangles(sampler, s_tri.LeftSplit(hc), triangles);
    right_child->GatherTriangles(sampler, s_tri.RightSplit(hc), triangles);
}

// void RoamTriangleNode::DebugPrint( size_t depth = 0) {
//    for (size_t i = 0; i < depth; ++i) {
//        log_file << "  ";
//    }

//    log_file << depth << " - rn: " << right_neighbor << " ln: " << left_neighbor << " bn: " << base_neighbor << " lc:" << left_child << " rc: " << right_child << endl;

//    if (left_child) {
//        left_child->DebugPrint( depth + 1 );
//    }
//    if (right_child) {
//        right_child->DebugPrint( depth + 1 );
//    }
// }
