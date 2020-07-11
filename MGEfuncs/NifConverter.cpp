#include "niflib.h"
#include "nif_math.h"

#include <assert.h>
#include <fstream>
#include <strstream>
#include <vector>
#include <float.h>
#include <map>

#include <d3d9.h>
#include <d3dx9.h>
#include "DXVertex.h"

#include "progmesh/ProgMesh.h"

#include "StaticType.h"
#include "ExportedNif.h"
#include "LandMesh.h"
#include "HeightFieldSampler.h"
#include "RoamLandPatch.h"

using namespace Niflib;

static IDirect3DDevice9* device;
static HANDLE staticFile;

extern "C" void TessellateLandscape( char* file_path, float* height_data, unsigned int data_height, unsigned int data_width, float top, float left, float bottom, float right, float error_tolerance) {

    if (top < bottom)
    {
        std::swap(top, bottom);
    }

    if (right < left)
    {
        std::swap(right, left);
    }

    // Create sampler
    HeightFieldSampler sampler( height_data, data_height, data_width, top, left, bottom, right );

    // Create patches
    float patch_width = 32768.0f;
    float patch_height = 32768.0f;

    // align patches the same regardless of actual world bounds
    float patches_bottom = floor(bottom / patch_height) * patch_height;
    float patches_top = ceil(top / patch_height) * patch_height;
    float patches_left = floor(left / patch_width) * patch_width;
    float patches_right = ceil(right / patch_width) * patch_width;

    size_t patches_down = (patches_top - patches_bottom) / patch_height;
    size_t patches_across = (patches_right - patches_left) / patch_width;

    vector<RoamLandPatch> patches;

    Vector3 corner(patches_left, patches_bottom, 0.0f);

    // Fill in patch data
    patches.resize( patches_across * patches_down );

    for (size_t y = 0; y < patches_down; ++y) {
        for (size_t x = 0; x < patches_across; ++x) {
            size_t i = y * patches_across + x;

            patches[i].sampler = &sampler;
            patches[i].left = corner.x;
            patches[i].right = corner.x + patch_width;
            patches[i].bottom = corner.y;
            patches[i].top = corner.y + patch_height;

            // Move the corner right for the next patch
            corner.x += patch_width;
        }
        // Move the corner up and back to the left edge for the next patch
        corner.y += patch_height;
        corner.x = patches_left;
    }

    // Connect neighboring patches
    for (size_t y = 0; y < patches_down; ++y) {
        for (size_t x = 0; x < patches_across; ++x) {
            size_t index = y * patches_across + x;
            size_t rn_index = y * patches_across + x + 1;
            size_t tn_index = (y+1) * patches_across + x;

            if (x != patches_across - 1) {
                patches[index].t2.left_neighbor = &patches[rn_index].t1;
                patches[rn_index].t1.left_neighbor = &patches[index].t2;
            }

            if (y != patches_down - 1) {
                patches[index].t2.right_neighbor = &patches[tn_index].t1;
                patches[tn_index].t1.right_neighbor = &patches[index].t2;
            }
        }
    }

    // Tesselate patches
    for (size_t i = 0; i < patches.size(); ++i) {
        patches[i].Tesselate( error_tolerance, 14 );
    }

    // patch.DebugPrint();

    // Generate Meshes
    vector<LandMesh> meshes;

    for (size_t i = 0; i < patches.size(); ++i) {
        LandMesh mesh = patches[i].GenerateMesh(16);

        if (mesh.vertices.size() > 0) {
            meshes.push_back( mesh );
        }
    }

    // Count verts and tris
    size_t verts = 0;
    size_t tris = 0;
    for (size_t i = 0; i < meshes.size(); ++i) {
        verts += meshes[i].vertices.size();
        tris += meshes[i].triangles.size();
    }

    // Save the Meshes
    LandMesh::SaveMeshes( file_path, meshes );
}


extern "C" int __stdcall GetVertSize() {
    return (int)sizeof(DXVertex);
}

extern "C" int __stdcall GetCompressedVertSize() {
    return (int)sizeof(DXCompressedVertex);
}

extern "C" int __stdcall GetLandVertSize() {
    return (int)sizeof(DXCompressedLandVertex);
}

extern "C" float __stdcall ProcessNif(char* data, int datasize, float simplify, float cutoff, BYTE static_type, BYTE old) {

    // Load the NIF data into our DirectX-friendly format
    ExportedNif nif;
    if (! nif.LoadNifFromStream(data, datasize)) {
        // log_file << "LoadNifFromStream failed." << endl;
        return -1;
    }

    // Calculate the bounds of the NIF to determine whether it exceeds our cutoff value
    nif.CalcBounds();

    if (static_type == STATIC_AUTO && nif.radius < cutoff) {
        // log_file << "Radius was below cutoff value." << endl;
        return -2;
    }

    // Buildings are treated as if they are twice their actual size.
    if (static_type == STATIC_BUILDING && nif.radius * 2.0f < cutoff) {
        // log_file << "Radius was below cutoff value." << endl;
        return -2;
    }

    if (!staticFile) {
        return nif.radius;
    }

    // Optimize NIF and calculate node bounds

    nif.Optimize(device, 16, simplify, old != 0);
    nif.CalcNodeBounds();

    // Determine whether this will be a near or far distant static based on size
    nif.static_type = static_type;

    // Save NIF to new format
    if (!nif.Save(staticFile)) {
        // log_file << "NIF Save failed." << endl;
        return -3;
    }

    return nif.radius;
}

extern "C" void __stdcall BeginStaticCreation(IDirect3DDevice9* _device, char* outpath) {
    device = _device;
    if (outpath) {
        staticFile = CreateFileA(outpath, FILE_GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    } else {
        staticFile = 0;
    }
}

extern "C" void __stdcall EndStaticCreation() {
    CloseHandle(staticFile);
}
