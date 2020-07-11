#include "nif_math.h"
#include "ExportedNode.h"
#include "progmesh/ProgMesh.h"

// Functions from OpenEXR to convert a float to a half float
static inline unsigned short FloatToHalfI(unsigned int i) {
    register int s = (i >> 16) & 0x00008000;
    register int e = ((i >> 23) & 0x000000ff) - (127 - 15);
    register int m = i & 0x007fffff;

    if (e <= 0) {
        if (e < -10) {
            return 0;
        }
        m = (m | 0x00800000) >> (1 - e);

        return s | (m >> 13);
    }
    else if (e == 0xff - (127 - 15)) {
        if (m == 0) { // Inf
            return s | 0x7c00;
        }
        else { // NAN
            m >>= 13;
            return s | 0x7c00 | m | (m == 0);
        }
    }
    else {
        if (e > 30) { // Overflow
            return s | 0x7c00;
        }

        return s | (e << 10) | (m >> 13);
    }
}

static inline unsigned short FloatToHalf(float i) {
    union {
        float f;
        unsigned int i;
    } v;
    v.f = i;
    return FloatToHalfI(v.i);
}

ExportedNode::ExportedNode() {
    center.Set(0.0f, 0.0f, 0.0f);
    radius = 0;
    verts = 0;
    faces = 0;
    vBuffer = 0;
    iBuffer = 0;
    emissive = 0.0f;
}

ExportedNode::ExportedNode(const ExportedNode& src) {
    center.Set(0.0f, 0.0f, 0.0f);
    radius = 0;
    verts = 0;
    faces = 0;
    vBuffer = 0;
    iBuffer = 0;
    emissive = 0.0f;

    *this = src;
}

ExportedNode::~ExportedNode() {
    SAFERELEASEP(vBuffer);
    SAFERELEASEP(iBuffer);
}

ExportedNode& ExportedNode::operator=(const ExportedNode& src) {
    SAFERELEASEP(vBuffer);
    SAFERELEASEP(iBuffer);

    verts = src.verts;
    faces = src.faces;
    tex = src.tex;
    emissive = src.emissive;

    if (verts) {
        vBuffer = new DXVertex[verts];
        memcpy(vBuffer, src.vBuffer, verts * sizeof(DXVertex));
    }

    if (faces) {
        iBuffer = new unsigned short[faces * 3];
        memcpy(iBuffer, src.iBuffer, faces * 6);
    }

    return *this;
}

void ExportedNode::CalcBounds() {
    // If the node has no vertices, give it default bounds
    if (verts == 0) {
        center.x = 0.0f;
        center.y = 0.0f;
        center.z = 0.0f;
        radius = 0.0f;
        return;
    }

    max = Niflib::Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    min = Niflib::Vector3(FLT_MAX, FLT_MAX, FLT_MAX);

    for (int v = 0; v < verts; ++v) {
        float x = vBuffer[v].Position.x;
        float y = vBuffer[v].Position.y;
        float z = vBuffer[v].Position.z;

        if (x > max.x) { max.x = x; }
        if (y > max.y) { max.y = y; }
        if (z > max.z) { max.z = z; }

        if (x < min.x) { min.x = x; }
        if (y < min.y) { min.y = y; }
        if (z < min.z) { min.z = z; }
    }

    // Store center of this node
    center = (min + max) / 2;

    // Find the furthest point from the center to get the radius
    float radius_squared = 0.0f;
    for (int v = 0; v < verts; ++v) {
        float x = vBuffer[v].Position.x;
        float y = vBuffer[v].Position.y;
        float z = vBuffer[v].Position.z;

        float dist_squared = (x - center.x) * (x - center.x) + (y - center.y) * (y - center.y) + (z - center.z) * (z - center.z);

        if (dist_squared > radius_squared) {
            radius_squared = dist_squared;
        }

    }

    // Store local radius of this node
    radius = sqrt(radius_squared);
}

void ExportedNode::Optimize(IDirect3DDevice9* device, unsigned int cache_size, float simplify, bool old) {
    const unsigned int stride = 36;

    if (simplify < 1 && faces > 8) {

        if (!old) {

            Niflib::ProgMesh pmesh(verts, faces, vBuffer, iBuffer);
            pmesh.ComputeProgressiveMesh();

            DXVertex* newVerts;
            WORD* newFaces;

            if (pmesh.DoProgressiveMesh(simplify, (DWORD*)&verts, (DWORD*)&faces, &newVerts, &newFaces) > 0) {
                delete[] iBuffer;
                delete[] vBuffer;
                iBuffer = newFaces;
                vBuffer = newVerts;

                /*char buf[260];
                sprintf(buf, "Mesh simplified from %d to %d", debugVerts, verts);
                OutputDebugStringA(buf);*/
            }

        }
        else {

            ID3DXMesh* mesh, * nmesh;
            DWORD* adjacency1 = new DWORD[faces * 3];
            DWORD* adjacency2 = new DWORD[faces * 3];
            void* data;

            D3DXATTRIBUTEWEIGHTS weights;
            memset(&weights, 0, sizeof(weights));
            weights.Boundary = 2;
            weights.Position = 1;
            weights.Normal = 1;

            // TODO: Check HRESULT
            D3DXCreateMeshFVF(faces, verts, D3DXMESH_SYSTEMMEM, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1, device, &mesh);
            mesh->LockVertexBuffer(0, &data);
            memcpy(data, vBuffer, verts * stride);
            mesh->UnlockVertexBuffer();
            mesh->LockIndexBuffer(0, &data);
            memcpy(data, iBuffer, faces * 6);
            mesh->UnlockIndexBuffer();

            mesh->GenerateAdjacency(0.0f, adjacency1);

            D3DXCleanMesh(D3DXCLEAN_SIMPLIFICATION, mesh, adjacency1, &nmesh, adjacency2, 0);
            mesh->Release();
            D3DXSimplifyMesh(nmesh, adjacency2, &weights, 0, (int)(faces * simplify), D3DXMESHSIMP_FACE, &mesh);
            nmesh->Release();

            delete[] adjacency1;
            delete[] adjacency2;

            verts = mesh->GetNumVertices();
            faces = mesh->GetNumFaces();

            delete[] vBuffer;
            delete[] iBuffer;

            vBuffer = new DXVertex[verts];
            iBuffer = new unsigned short[faces * 3];

            mesh->LockVertexBuffer(D3DLOCK_READONLY, &data);
            memcpy(vBuffer, data, verts * stride);
            mesh->UnlockVertexBuffer();
            mesh->LockIndexBuffer(D3DLOCK_READONLY, &data);
            memcpy(iBuffer, data, faces * 6);
            mesh->UnlockIndexBuffer();

            mesh->Release();

        }

    }

    // Create temporary 32-bit index buffer
    unsigned int* iBuffer32 = new unsigned int[faces * 3];
    for (int j = 0; j < faces * 3; ++j) {
        iBuffer32[j] = iBuffer[j];
    }


    TootleResult result = TootleOptimizeVCache(iBuffer32, faces, verts, cache_size, iBuffer32, NULL, TOOTLE_VCACHE_AUTO);

    if (result != TOOTLE_OK) {
        // log_file << "TootleOptimizeVCache returned an error" << endl;
        delete[] iBuffer32;
        return;
    }

    result = TootleOptimizeVertexMemory(vBuffer, iBuffer32, verts, faces, stride, vBuffer, iBuffer32, NULL);

    if (result != TOOTLE_OK) {
        // log_file << "TootleOptimizeVertexMemory returned an error" << endl;
        delete[] iBuffer32;
        return;
    }

    // Copy 32-bit index buffer back into 16-bit indices
    for (int j = 0; j < faces * 3; ++j) {
        iBuffer[j] = (unsigned short)iBuffer32[j];
    }

    // free temporary buffer
    delete[] iBuffer32;
}


void ExportedNode::Save(HANDLE& file) {
    DWORD unused;
    short slen = (short)tex.size() + 1;

    // Write radius and center
    WriteFile(file, &radius, 4, &unused, 0);

    WriteFile(file, &center.x, 4, &unused, 0);
    WriteFile(file, &center.y, 4, &unused, 0);
    WriteFile(file, &center.z, 4, &unused, 0);

    // Write min and max (bounding box)
    WriteFile(file, &min.x, 4, &unused, 0);
    WriteFile(file, &min.y, 4, &unused, 0);
    WriteFile(file, &min.z, 4, &unused, 0);

    WriteFile(file, &max.x, 4, &unused, 0);
    WriteFile(file, &max.y, 4, &unused, 0);
    WriteFile(file, &max.z, 4, &unused, 0);

    // Write vert and face counts
    WriteFile(file, &verts, 4, &unused, 0);
    WriteFile(file, &faces, 4, &unused, 0);

    // Compress vertex buffer
    DXCompressedVertex* compVBuf = new DXCompressedVertex[verts];
    for (int i = 0; i < verts; ++i) {
        DXCompressedVertex& cv = compVBuf[i];
        DXVertex& v = vBuffer[i];

        // Copy uncompressed values
        cv.Diffuse[0] = v.Diffuse[0];
        cv.Diffuse[1] = v.Diffuse[1];
        cv.Diffuse[2] = v.Diffuse[2];
        cv.Diffuse[3] = v.Diffuse[3];

        // Compress position
        cv.Position[0] = FloatToHalf(v.Position.x);
        cv.Position[1] = FloatToHalf(v.Position.y);
        cv.Position[2] = FloatToHalf(v.Position.z);
        cv.Position[3] = FloatToHalf(1.0f);

        // Compress Texcoords
        cv.texCoord[0] = FloatToHalf(v.texCoord.u);
        cv.texCoord[1] = FloatToHalf(v.texCoord.v);

        // Compress normals
        cv.Normal[0] = (unsigned char)(255.0f * (v.Normal.x * 0.5 + 0.5));
        cv.Normal[1] = (unsigned char)(255.0f * (v.Normal.y * 0.5 + 0.5));
        cv.Normal[2] = (unsigned char)(255.0f * (v.Normal.z * 0.5 + 0.5));
        cv.Normal[3] = (unsigned char)(255.0f * emissive);
    }

    // Write vertex and index buffers
    WriteFile(file, compVBuf, verts * sizeof(DXCompressedVertex), &unused, 0);
    WriteFile(file, iBuffer, faces * 6, &unused, 0);

    // Delete compressed vertices
    delete[] compVBuf;

    // Write texture name
    WriteFile(file, &slen, 2, &unused, 0);
    WriteFile(file, tex.c_str(), slen, &unused, 0);
}

void ExportedNode::Load(HANDLE& file) {
    DWORD unused;

    // Read radius and center
    ReadFile(file, &radius, 4, &unused, 0);

    ReadFile(file, &center.x, 4, &unused, 0);
    ReadFile(file, &center.y, 4, &unused, 0);
    ReadFile(file, &center.z, 4, &unused, 0);

    // Read min and max (bounding box)
    ReadFile(file, &min.x, 4, &unused, 0);
    ReadFile(file, &min.y, 4, &unused, 0);
    ReadFile(file, &min.z, 4, &unused, 0);

    ReadFile(file, &max.x, 4, &unused, 0);
    ReadFile(file, &max.y, 4, &unused, 0);
    ReadFile(file, &max.z, 4, &unused, 0);

    // Read vert and face counts
    ReadFile(file, &verts, 4, &unused, 0);
    ReadFile(file, &faces, 4, &unused, 0);

    // Read vertex and index buffers
    ReadFile(file, vBuffer, verts * sizeof(DXCompressedVertex), &unused, 0);
    ReadFile(file, iBuffer, faces * 6, &unused, 0);

    // Read texture name
    short slen;
    ReadFile(file, &slen, 2, &unused, 0);

    char* tmp = new char[slen];
    ReadFile(file, tmp, slen, &unused, 0);
    tex = tmp;
    delete[] tmp;
}
