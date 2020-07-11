#include "ExportedNif.h"

using namespace Niflib;

void ExportedNif::CalcBounds() {
    // Calculate the total bounds of all nodes
    Vector3 max = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    Vector3 min = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);

    // Find minimum and maximum x, y, and z positions
    for (size_t n = 0; n < nodes.size(); ++n) {
        // If the node has no vertices, give it default bounds
        if (nodes[n].verts == 0) {
            nodes[n].center.x = 0.0f;
            nodes[n].center.y = 0.0f;
            nodes[n].center.z = 0.0f;
            nodes[n].radius = 0.0f;
            continue;
        }

        for (int v = 0; v < nodes[n].verts; ++v) {
            float x, y, z;
            x = nodes[n].vBuffer[v].Position.x;
            y = nodes[n].vBuffer[v].Position.y;
            z = nodes[n].vBuffer[v].Position.z;

            if (x > max.x) { max.x = x; }
            if (y > max.y) { max.y = y; }
            if (z > max.z) { max.z = z; }

            if (x < min.x) { min.x = x; }
            if (y < min.y) { min.y = y; }
            if (z < min.z) { min.z = z; }
        }
    }

    // Average min/max positions to get center
    center = (min + max) / 2;

    // Find the furthest point from the center to get the radius
    float radius_squared = 0.0f;
    for (size_t n = 0; n < nodes.size(); ++n) {
        for (int v = 0; v < nodes[n].verts; ++v) {
            float x = nodes[n].vBuffer[v].Position.x;
            float y = nodes[n].vBuffer[v].Position.y;
            float z = nodes[n].vBuffer[v].Position.z;

            float dist_squared = (x - center.x) * (x - center.x) + (y - center.y) * (y - center.y) + (z - center.z) * (z - center.z);

            if (dist_squared > radius_squared) {
                radius_squared = dist_squared;
            }
        }
    }

    radius = sqrt(radius_squared);
}

void ExportedNif::CalcNodeBounds() {
    for (size_t i = 0; i < nodes.size(); ++i) {
        nodes[i].CalcBounds();
    }
}

bool ExportedNif::MergeShape(ExportedNode* dst, ExportedNode* src) {
    // Sum vert and face counts
    int verts = src->verts + dst->verts;
    int faces = src->faces + dst->faces;

    // Create new buffers large enough to hold all vertices from original ones
    DXVertex* v_buf = new DXVertex[verts];
    unsigned short* i_buf = new unsigned short[faces * 3];

    // Copy data from previous buffers into new ones
    memcpy(v_buf, dst->vBuffer, dst->verts * sizeof(DXVertex));
    memcpy(v_buf + dst->verts, src->vBuffer, src->verts * sizeof(DXVertex));

    memcpy(i_buf, dst->iBuffer, dst->faces * 6);
    memcpy(i_buf + dst->faces * 3, src->iBuffer, src->faces * 6);

    // Account for the offset in the indices copied from src
    for (int i = dst->faces * 3; i < faces * 3; ++i) {
        i_buf[i] += dst->verts;
    }

    // Delete old dst buffers
    delete[] dst->vBuffer;
    delete[] dst->iBuffer;

    // Set new values in dst
    dst->vBuffer = v_buf;
    dst->verts = verts;
    dst->iBuffer = i_buf;
    dst->faces = faces;

    return true;
}

void ExportedNif::SearchShapes(NiAVObjectRef rootObj, vector<NiTriBasedGeomRef>* SubsetNodes) {
    // Check if this object is derived from NiTriBasedGeom
    NiTriBasedGeomRef niGeom = DynamicCast<NiTriBasedGeom>(rootObj);
    if (niGeom) {
        SubsetNodes->push_back(niGeom);
    }
    else {
        // Check if this object derives from NiNode and, thus, may have children
        // Follow switch index for NiSwitchNodes, ignore RootCollisionNodes
        NiNodeRef niNode = DynamicCast<NiNode>(rootObj);
        NiSwitchNodeRef niSwitch = DynamicCast<NiSwitchNode>(rootObj);
        RootCollisionNodeRef collision = DynamicCast<RootCollisionNode>(rootObj);
        if (niSwitch) {
            // Search just the selected child
            SearchShapes(niSwitch->GetActiveChild(), SubsetNodes);
        }
        else if (niNode && !collision) {
            // Call this function for any children
            vector<NiAVObjectRef> children = niNode->GetChildren();
            for (size_t i = 0; i < children.size(); i++) {
                SearchShapes(children[i], SubsetNodes);
            }
        }
    }
}

bool ExportedNif::ExportShape(NiTriBasedGeomRef niGeom, ExportedNode* node) {
    // Check that an external texture exists
    NiTexturingPropertyRef niTexProp = DynamicCast<NiTexturingProperty>(niGeom->GetPropertyByType(NiTexturingProperty::TYPE));
    if (!niTexProp || niTexProp->GetTextureCount() == 0) {
        // log_file << "External texture does not exist" << endl;
        return false;
    }
    TexDesc texDesc = niTexProp->GetTexture(0);
    NiSourceTextureRef niSrcTex = texDesc.source;
    if (!niSrcTex || !niSrcTex->IsTextureExternal()) {
        // log_file << "Texture was not external" << endl;
        return false;
    }

    // Get data object (NiTriBasedGeomData) from geometry node (NiTriBasedGeom)
    NiTriBasedGeomDataRef niGeomData = DynamicCast<NiTriBasedGeomData>(niGeom->GetData());
    if (!niGeomData) {
        // log_file << "There is no Geometry data on this mesh." << endl;
        return false;
    }

    // Get material object (NiMaterialProperty) from geometry node (NiTriBasedGeom)
    NiMaterialPropertyRef niMatProp = DynamicCast<NiMaterialProperty>(niGeom->GetPropertyByType(NiMaterialProperty::TYPE));

    // Get Diffuse color (will be baked into vertices
    Color3 diffuse(1.0f, 1.0f, 1.0f);
    if (niMatProp) {
        diffuse = niMatProp->GetDiffuseColor();
    }
    // Get the emissive color (will be averaged and stored in alpha chanel of vertices)
    Color3 emissive(0.0f, 0.0f, 0.0f);
    if (niMatProp) {
        emissive = niMatProp->GetEmissiveColor();
    }
    node->emissive = (emissive.r + emissive.b + emissive.g) / 3.0f;

    // Get the alpha of the material
    float Alpha = 1.0f;
    if (niMatProp) {
        Alpha = niMatProp->GetTransparency();
    }

    // Check that there is at least one set of texture coords available
    if (niGeomData->GetUVSetCount() == 0) {
        // log_file << "There are no texture coordinates on this mesh." << endl;
        return false;
    }

    // ProgMesh simplify = ProgMesh(niGeomData);
    // simplify.ComputeProgressiveMesh();
    // simplify.DoProgressiveMesh(0.1);

    // Indicies
    vector<Triangle> tris = niGeomData->GetTriangles();
    node->faces = tris.size();
    if (node->faces == 0) {
        // log_file << "This mesh has no triangles." << endl;
        return false;
    }

    // Now that we're sure this mesh is valid, start the conversion

    // Get transformation of mesh as 4x4 matrix
    Matrix44 transform = niGeom->GetWorldTransform();

    // Get a matrix that only contains the world rotation matrix
    // This will be used to transform normals
    Matrix44 rotation(transform.GetRotation());

    node->iBuffer = new unsigned short[node->faces * 3];
    for (int i = 0; i < node->faces; i++) {
        node->iBuffer[i * 3 + 0] = tris[i].v1;
        node->iBuffer[i * 3 + 1] = tris[i].v2;
        node->iBuffer[i * 3 + 2] = tris[i].v3;
    }

    // Vertex data
    vector<Vector3> positions;
    vector<Vector3> normals;
    if (niGeom->IsSkin()) {
        niGeom->GetSkinDeformation(positions, normals);
    }
    else {
        positions = niGeomData->GetVertices();
        normals = niGeomData->GetNormals();
    }
    vector<Color4> colors = niGeomData->GetColors();
    vector<TexCoord> texCoords = niGeomData->GetUVSet(0);

    // Verticies
    node->verts = niGeomData->GetVertexCount();

    node->vBuffer = new DXVertex[node->verts];
    for (int i = 0; i < node->verts; i++) {
        // Push the world transform into the verticies
        positions[i] = transform * positions[i];

        normals[i] = rotation * normals[i];

        // Apply the world transform's rotation to the normals

        node->vBuffer[i].Position = positions[i];
        if (normals.size() > 0) {
            node->vBuffer[i].Normal = normals[i];
        }
        else {
            node->vBuffer[i].Normal.x = 0;
            node->vBuffer[i].Normal.y = 0;
            node->vBuffer[i].Normal.z = 1;
        }
        if (colors.size() > 0) {
            node->vBuffer[i].Diffuse[0] = (unsigned char)(255.0f * colors[i].b * diffuse.b);
            node->vBuffer[i].Diffuse[1] = (unsigned char)(255.0f * colors[i].g * diffuse.g);
            node->vBuffer[i].Diffuse[2] = (unsigned char)(255.0f * colors[i].r * diffuse.r);
            node->vBuffer[i].Diffuse[3] = (unsigned char)(255.0f * colors[i].a * Alpha);
        }
        else {
            node->vBuffer[i].Diffuse[0] = (unsigned char)(255.0f * diffuse.b);
            node->vBuffer[i].Diffuse[1] = (unsigned char)(255.0f * diffuse.g);
            node->vBuffer[i].Diffuse[2] = (unsigned char)(255.0f * diffuse.r);
            node->vBuffer[i].Diffuse[3] = (unsigned char)(255.0f * Alpha);
        }

        node->vBuffer[i].texCoord = texCoords[i];
    }

    // Get texture file path
    string s = niSrcTex->GetTextureFileName();

    // Make texture path all lowercase
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] >= 65 && s[i] <= 90) {
            s[i] += 32;
        }
    }

    // If the path starts with "textures" or "\textures" remove it
    size_t pos = s.find("textures");
    if (pos <= 1) {
        s = s.substr(pos + 8, string::npos);
    }

    // Remove any leading forward slashes that remain
    if (s[0] == '\\') {
        s = s.substr(1, string::npos);
    }

    node->tex = s;

    return true;
}

void ExportedNif::Optimize(IDirect3DDevice9* device, unsigned int cache_size, float simplify, bool old) {
    // Try to combine nodes that have the same texture path
    map<string, ExportedNode*> node_tex;

    for (size_t i = 0; i < nodes.size(); ++i) {
        // Check if this node has already been found
        map<string, ExportedNode*>::iterator it = node_tex.find(nodes[i].tex);

        if (it == node_tex.end()) {
            // Nothing with this texture has been found yet.  Store the node's pointer in the map
            node_tex[nodes[i].tex] = &nodes[i];
        }
        else {
            // A shape with this texture has been found already.  Merge this one into it.
            MergeShape(it->second, &nodes[i]);
        }
    }

    size_t count = 0;
    if (node_tex.size() < nodes.size() && node_tex.size() != 0) {
        // We reduced the number of nodes, so create a new list to save
        vector<ExportedNode> merged_nodes(node_tex.size());

        for (map<string, ExportedNode*>::iterator it = node_tex.begin(); it != node_tex.end(); ++it) {
            merged_nodes[count] = *(it->second);

            ++count;
        }

        nodes = merged_nodes;
    }


    // Now optimize each node
    for (size_t i = 0; i < nodes.size(); ++i) {
        nodes[i].Optimize(device, cache_size, simplify, old);
    }
}

bool ExportedNif::LoadNifFromStream(const char* data, int size) {
    istrstream s(data, size);
    NiAVObjectRef rootObj;

    try {
        rootObj = DynamicCast<NiAVObject>(ReadNifTree(s, 0));
    }
    catch (std::runtime_error& e) {
        std::fstream error_log("distant-land-niflib-error.log", std::ios_base::out | std::ios_base::app);
        error_log << e.what() << std::endl;
        return false;
    }

    if (!rootObj) {
        // log_file << "Root object was null." << endl;
        return false;
    }

    // Object root transform should not affect results
    rootObj->SetLocalTransform(Matrix44::IDENTITY);

    vector<NiTriBasedGeomRef> SubsetNodes;
    SearchShapes(rootObj, &SubsetNodes);

    if (SubsetNodes.size() == 0) {
        // log_file << "SubsetNodes size is zero." << endl;
        return false;
    }

    for (size_t i = 0; i < SubsetNodes.size(); ++i) {
        ExportedNode tmp_node;
        if (ExportShape(SubsetNodes[i], &tmp_node)) {
            nodes.push_back(tmp_node);
        }
    }

    if (nodes.size() == 0) {
        // log_file << "nodes size is zero." << endl;
        return false;
    }

    // Success
    return true;
}

bool ExportedNif::Save(HANDLE file) {
    // HANDLE h = CreateFileA(outpath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    HANDLE h = file;

    DWORD unused;
    if (h == INVALID_HANDLE_VALUE) {
        // log_file << "File handle is invalid." << endl;
        return false;
    }
    int nodeCount = (int)nodes.size();
    SetFilePointer(h, 0, NULL, FILE_END);

    WriteFile(h, &nodeCount, 4, &unused, 0);
    WriteFile(h, &radius, 4, &unused, 0);
    WriteFile(h, &center.x, 4, &unused, 0);
    WriteFile(h, &center.y, 4, &unused, 0);
    WriteFile(h, &center.z, 4, &unused, 0);
    WriteFile(h, &static_type, 1, &unused, 0);

    for (size_t i = 0; i < nodes.size(); i++) {
        nodes[i].Save(h);
    }

    return true;
}
