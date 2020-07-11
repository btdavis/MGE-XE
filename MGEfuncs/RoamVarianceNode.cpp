#include "RoamVarianceNode.h"
#include "HeightFieldSampler.h"

using namespace Niflib;

size_t RoamVarianceNode::last_used_index = 0;
vector<RoamVarianceNode> RoamVarianceNode::pool(32768);

RoamVarianceNode* RoamVarianceNode::Create() {
    // Make sure pool can hold this object
    if (last_used_index + 1 >= pool.size()) {
        // No more room RoamVarianceNode objects to hand out.
        return nullptr;
    }

    // increment the last used index and return the object at that position
    ++last_used_index;
    // Clear out any data in the object so it's as if it's a new object
    pool[last_used_index].ClearChildren();

    return &pool[last_used_index];
}

// This shouldn't be called directly, used Create instead to allocate with the pool
RoamVarianceNode::RoamVarianceNode() : variance(0.0f), left_child(0), right_child(0) {}

void RoamVarianceNode::ResetPool() {
    // Move the index of the next node to be handed out back to the begining.
    // There should be no outstanding RoamVarianceNode pointers when this is called.
    last_used_index = 0;
}

void RoamVarianceNode::ClearChildren() {
    left_child = 0;
    right_child = 0;
}

RoamVarianceNode::~RoamVarianceNode() {}

void RoamVarianceNode::CalculateVariance(HeightFieldSampler* sampler, const SplitTriangle& tri, size_t depth) {
    // On the downward pass, calculate the variance as the difference in height between the
    // average of left and right, and the the real height value as given by the sampler
    Vector3 avg = tri.GetHypoCenter();
    float samp_height = sampler->SampleHeight(avg.x, avg.y);

    variance = abs(avg.z - samp_height);

    // Give extra weight to the split if it causes the vertex to switch from being above the water to being below the water or vice versa
    // Water level is zero
    if ((avg.z > 0.0f && samp_height < 0.0f) || (samp_height > 0.0f && avg.z < 0.0f)) {
        variance *= 4.0f;
    }

    // Give extra weight to the split if it the average vertex would be above the real height
    if (avg.z > samp_height) {
        variance *= 4.0f;
    }

    // If we have reached the maximum depth, free any children and start unwiding traversal
    if (depth == 0) {
        ClearChildren();
        return;
    }

    // If we have not reached the maximum depth, create children and call this function on them
    avg.z = samp_height;

    if (!right_child) {
        right_child = Create();
    }
    right_child->CalculateVariance(sampler, tri.RightSplit(avg), depth - 1);


    if (!left_child) {
        left_child = Create();
    }
    left_child->CalculateVariance(sampler, tri.LeftSplit(avg), depth - 1);

    // We want the variance of this node to represent the maximum variance of all children nodes, so choose the highest of
    // the variances as the new variance
    if (right_child->variance > variance) {
        variance = right_child->variance;
    }
    if (left_child->variance > variance) {
        variance = left_child->variance;
    }
}
