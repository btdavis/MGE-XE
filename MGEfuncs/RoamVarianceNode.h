#ifndef ROAMVARIANCENODE_H
#define ROAMVARIANCENODE_H

#include "HeightFieldSampler.h"
#include "SplitTriangle.h"

class RoamVarianceNode {
public:
    float variance;
    RoamVarianceNode* left_child;
    RoamVarianceNode* right_child;

private:
    static size_t last_used_index;
    static vector<RoamVarianceNode> pool;


public:
    static RoamVarianceNode* Create();
    static void ResetPool();

    // This shouldn't be called directly, used Create instead to allocate with the pool
    RoamVarianceNode();
    ~RoamVarianceNode();

    void ClearChildren();

    void CalculateVariance(HeightFieldSampler* sampler, const SplitTriangle& tri, size_t depth);
};

#endif
