#include "RoamLandPatch.h"
#include "RoamVarianceNode.h"
#include "RenderTriangle.h"

using namespace Niflib;

class CmpVec3 : public Vector3 {
public:
    CmpVec3(const Vector3& v) {
        x = v.x;
        y = v.y;
        z = v.z;
    }

    CmpVec3& operator=(const Vector3& rh) {
        x = rh.x;
        y = rh.y;
        z = rh.z;
        return *this;
    }

    Vector3 AsVec3() {
        return Vector3(x, y, z);
    }

    static bool FltEq(float lh, float rh) {
        return abs(lh - rh) < 0.0001f;
    }

    static bool FltLt(float lh, float rh) {
        return lh - 0.0001f < rh;
    }

    bool operator==(const CmpVec3& rh) const {
        return FltEq(x, rh.x) && FltEq(y, rh.y) && FltEq(z, rh.z);
    }

    friend bool operator<(const CmpVec3& lh, const CmpVec3& rh) {
        if (lh == rh) {
            return false;
        }

        if (FltEq(lh.x, rh.x)) {
            if (FltEq(lh.y, rh.y)) {
                return lh.z < rh.z;
            }
            else {
                return lh.y < rh.y;
            }
        }

        return lh.x < rh.x;
    }
};

RoamLandPatch::RoamLandPatch() {
    t1.base_neighbor = &t2;
    t2.base_neighbor = &t1;
}

RoamLandPatch::RoamLandPatch(float t, float l, float b, float r, HeightFieldSampler* s) : top(t), left(l), bottom(b), right(r), sampler(s) {
    t1.base_neighbor = &t2;
    t2.base_neighbor = &t1;
}

RoamLandPatch::RoamLandPatch(const RoamLandPatch& patch) {
    sampler = patch.sampler;
    top = patch.top;
    left = patch.left;
    bottom = patch.bottom;
    right = patch.right;

    t1.base_neighbor = &t2;
    t2.base_neighbor = &t1;
}

RoamLandPatch::~RoamLandPatch() {}

Vector3 RoamLandPatch::GetTopLeft() {
    return Vector3(left, top, sampler->SampleHeight(left, top));
}
Vector3 RoamLandPatch::GetBottomLeft() {
    return Vector3(left, bottom, sampler->SampleHeight(left, bottom));
}
Vector3 RoamLandPatch::GetTopRight() {
    return Vector3(right, top, sampler->SampleHeight(right, top));
}
Vector3 RoamLandPatch::GetBottomRight() {
    return Vector3(right, bottom, sampler->SampleHeight(right, bottom));
}

void RoamLandPatch::ConnectRightNeighbor(RoamLandPatch& neighbor) {
    t2.right_neighbor = &neighbor.t1;
    neighbor.t1.right_neighbor = &t2;
}

void RoamLandPatch::ConnectLeftNeighbor(RoamLandPatch& neighbor) {
    t1.right_neighbor = &neighbor.t2;
    neighbor.t2.right_neighbor = &t1;
}

void RoamLandPatch::ConnectTopNeighbor(RoamLandPatch& neighbor) {
    t2.left_neighbor = &neighbor.t1;
    neighbor.t1.left_neighbor = &t2;
}

void RoamLandPatch::ConnectBottomNeighbor(RoamLandPatch& neighbor) {
    t1.left_neighbor = &neighbor.t2;
    neighbor.t2.left_neighbor = &t1;
}

void RoamLandPatch::Tesselate(float max_variance, size_t tree_depth) {
    Vector3 top_left = GetTopLeft();
    Vector3 bottom_left = GetBottomLeft();
    Vector3 top_right = GetTopRight();
    Vector3 bottom_right = GetBottomRight();

    RoamVarianceNode* variance_root = RoamVarianceNode::Create();
    variance_root->CalculateVariance(sampler, SplitTriangle(bottom_right, top_left, bottom_left), tree_depth);
    t1.Tesselate(variance_root, max_variance);
    variance_root = 0;
    RoamVarianceNode::ResetPool();

    variance_root = RoamVarianceNode::Create();
    variance_root->CalculateVariance(sampler, SplitTriangle(top_left, bottom_right, top_right), tree_depth);
    t2.Tesselate(variance_root, max_variance);
    variance_root = 0;

    RoamVarianceNode::ResetPool();
}

LandMesh RoamLandPatch::GenerateMesh(unsigned int cache_size) {
    LandMesh mesh;
    Vector3 top_left = GetTopLeft();
    Vector3 bottom_left = GetBottomLeft();
    Vector3 top_right = GetTopRight();
    Vector3 bottom_right = GetBottomRight();

    vector<RenderTriangle> render_triangles;

    t1.GatherTriangles(sampler, SplitTriangle(bottom_right, top_left, bottom_left), render_triangles);
    t2.GatherTriangles(sampler, SplitTriangle(top_left, bottom_right, top_right), render_triangles);

    // Walk through the triangles that were gathered, adding their vertices and indices to a map
    Vector3 max = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    Vector3 min = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);

    map<CmpVec3, size_t> vert_map;
    size_t render_triangles_size = render_triangles.size();
    for (size_t i = 0; i < render_triangles_size; ++i) {
        size_t right_index, left_index, top_index;
        map<CmpVec3, size_t>::iterator it;

        it = vert_map.find(render_triangles[i].st.left);
        if (it == vert_map.end()) {
            // Add new index
            left_index = mesh.vertices.size();
            Vector3& v = render_triangles[i].st.left;
            vert_map[v] = left_index;
            mesh.vertices.push_back(v);

            if (v.x > max.x) { max.x = v.x; }
            if (v.y > max.y) { max.y = v.y; }
            if (v.z > max.z) { max.z = v.z; }

            if (v.x < min.x) { min.x = v.x; }
            if (v.y < min.y) { min.y = v.y; }
            if (v.z < min.z) { min.z = v.z; }
        }
        else {
            // Get index from existing vertex
            left_index = it->second;
        }

        it = vert_map.find(render_triangles[i].st.right);
        if (it == vert_map.end()) {
            // Add new index
            right_index = mesh.vertices.size();
            Vector3& v = render_triangles[i].st.right;
            vert_map[v] = right_index;
            mesh.vertices.push_back(v);

            if (v.x > max.x) { max.x = v.x; }
            if (v.y > max.y) { max.y = v.y; }
            if (v.z > max.z) { max.z = v.z; }

            if (v.x < min.x) { min.x = v.x; }
            if (v.y < min.y) { min.y = v.y; }
            if (v.z < min.z) { min.z = v.z; }
        }
        else {
            // Get index from existing vertex
            right_index = it->second;
        }

        it = vert_map.find(render_triangles[i].st.top);
        if (it == vert_map.end()) {
            // Add new index
            top_index = mesh.vertices.size();
            Vector3& v = render_triangles[i].st.top;
            vert_map[v] = top_index;
            mesh.vertices.push_back(v);

            if (v.x > max.x) { max.x = v.x; }
            if (v.y > max.y) { max.y = v.y; }
            if (v.z > max.z) { max.z = v.z; }

            if (v.x < min.x) { min.x = v.x; }
            if (v.y < min.y) { min.y = v.y; }
            if (v.z < min.z) { min.z = v.z; }
        }
        else {
            // Get index from existing vertex
            top_index = it->second;
        }

        mesh.triangles.push_back(LargeTriangle(right_index, top_index, left_index));
    }

    if (abs(min.z + 2048.0f) <= 0.001f && abs(max.z + 2048.0f) <= 0.001f) {
        // This mesh is perfectly flat at the bottom of the world.  Return an empty mesh.
        return LandMesh();
    }

    // Calculate mesh bounds
    mesh.CalcBounds(min, max);

    // Cache optimize triangle and vertex order
    unsigned int* iBuffer = (unsigned int*)&mesh.triangles[0];
    void* vBuffer = (void*)&mesh.vertices[0];
    unsigned int verts = (unsigned int)mesh.vertices.size();
    unsigned int faces = (unsigned int)mesh.triangles.size();
    unsigned int stride = 3 * sizeof(float);

    TootleResult result = TootleOptimizeVCache(iBuffer, faces, verts, cache_size, iBuffer, NULL, TOOTLE_VCACHE_AUTO);

    if (result != TOOTLE_OK) {
        // log_file << "TootleOptimizeVCache returned an error" << endl;
        return LandMesh();
    }

    result = TootleOptimizeVertexMemory(vBuffer, iBuffer, verts, faces, stride, vBuffer, iBuffer, NULL);

    if (result != TOOTLE_OK) {
        // log_file << "TootleOptimizeVertexMemory returned an error" << endl;
        return LandMesh();
    }

    // Now that all the vertices have been found, figure out the UVs for them
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        mesh.uvs.push_back(sampler->SampleTexCoord(mesh.vertices[i].x, mesh.vertices[i].y));
    }

    return mesh;
}

// void RoamLandPatch::DebugPrint() {
//    t1.DebugPrint();
//    t2.DebugPrint();
// }
