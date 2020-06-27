#ifndef EXPORTEDNIF_H
#define EXPORTEDNIF_H

#include "ExportedNode.h"
#include "niflib.h"
#include "obj/NiAVObject.h"
#include "obj/NiTriBasedGeom.h"
#include <vector>

class ExportedNif {
public:
    Niflib::Vector3 center;
    float radius;
    unsigned char static_type;
    vector<ExportedNode> nodes;

    void CalcBounds();
    void CalcNodeBounds();
    void Optimize(IDirect3DDevice9* device, unsigned int cache_size, float simplify, bool old);
    bool LoadNifFromStream(const char* data, int size);
    bool Save(HANDLE file);

private:
    bool MergeShape(ExportedNode* dst, ExportedNode* src);
    void SearchShapes(Niflib::NiAVObjectRef rootObj, vector<Niflib::NiTriBasedGeomRef>* SubsetNodes);
    bool ExportShape(Niflib::NiTriBasedGeomRef niGeom, ExportedNode* node);
};

#endif
