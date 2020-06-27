#ifndef EXPORTEDNODE_H
#define EXPORTEDNODE_H

#include "DXVertex.h"
#include "nif_math.h"

struct ExportedNode {
    Niflib::Vector3 center;
    float radius;
    Niflib::Vector3 max;
    Niflib::Vector3 min;
    int verts;
    int faces;
    DXVertex* vBuffer;
    unsigned short* iBuffer;
    string tex;
    float emissive;

    ExportedNode();
    ExportedNode(const ExportedNode& src);
    ~ExportedNode();

    ExportedNode& operator=(const ExportedNode& src);

    void CalcBounds();
    void Optimize(IDirect3DDevice9* device, unsigned int cache_size, float simplify, bool old);
    void Save(HANDLE& file);
    void Load(HANDLE& file);
};

#endif
