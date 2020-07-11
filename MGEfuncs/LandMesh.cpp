#include "LandMesh.h"
#include "DXVertex.h"

using namespace Niflib;

void LandMesh::CalcBounds(const Vector3& new_min, const Vector3& new_max) {

    min = new_min;
    max = new_max;

    // Average min/max positions to get center
    center = (min + max) / 2.0f;

    // Find the furthest point from the center to get the radius
    float radius_sqared = 0.0f;

    for (size_t i = 0; i < vertices.size(); ++i) {
        float x, y, z;
        x = vertices[i].x;
        y = vertices[i].y;
        z = vertices[i].z;

        float dist = (x - center.x) * (x - center.x) + (y - center.y) * (y - center.y) + (z - center.z) * (z - center.z);

        if (dist > radius_sqared) {
            radius_sqared = dist;
        }
    }

    radius = sqrt(radius_sqared);
}

bool LandMesh::SaveMeshes(LPCSTR file_path, vector<LandMesh>& meshes) {
    DWORD mesh_count, unused;
    HANDLE file = CreateFile(file_path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }
    mesh_count = meshes.size();
    WriteFile(file, &mesh_count, 4, &unused, 0);

    for (size_t i = 0; i < meshes.size(); ++i) {
        if (!meshes[i].Save(file)) {
            return false;
        }
    }

    CloseHandle(file);

    return true;
}

bool LandMesh::Save(HANDLE& file) {
    DWORD verts, faces, unused;
    bool large;
    verts = vertices.size();
    faces = triangles.size();

    WriteFile(file, &radius, 4, &unused, 0);
    WriteFile(file, &center, 12, &unused, 0);

    WriteFile(file, &min, 12, &unused, 0);
    WriteFile(file, &max, 12, &unused, 0);

    WriteFile(file, &verts, 4, &unused, 0);
    WriteFile(file, &faces, 4, &unused, 0);

    if (verts > 0xFFFF || faces > 0xFFFF) {
        large = true;
    }
    else {
        large = false;
    }

    DXCompressedLandVertex* compVerts = new DXCompressedLandVertex[verts];

    for (size_t i = 0; i < verts; ++i) {
        compVerts[i].Position = vertices[i];

        // WriteFile(file,&vertices[i], 12, &unused, 0);
        // WriteFile(file,&uvs[i], 8, &unused, 0);

        // Compress UVs to a normalized 16-bit unsigned short
        // short compUVs[2];
        compVerts[i].texCoord[0] = (short)(uvs[i].u * 32768.0f);
        compVerts[i].texCoord[1] = (short)(uvs[i].v * 32768.0f);
        // WriteFile(file, compUVs, 4, &unused, 0);
    }

    WriteFile(file, compVerts, sizeof(DXCompressedLandVertex) * verts, &unused, 0);

    delete[] compVerts;

    for (size_t i = 0; i < faces; ++i) {
        if (large == true) {
            unsigned int tmp[3];
            tmp[0] = triangles[i].v1;
            tmp[1] = triangles[i].v2;
            tmp[2] = triangles[i].v3;
            WriteFile(file, tmp, 4 * 3, &unused, 0);
        }
        else {
            unsigned short tmp[3];
            tmp[0] = triangles[i].v1;
            tmp[1] = triangles[i].v2;
            tmp[2] = triangles[i].v3;
            WriteFile(file, tmp, 2 * 3, &unused, 0);
        }
    }

    return true;
}
