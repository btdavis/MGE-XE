#ifndef COLLAPSETRIANGLE_H
#define COLLAPSETRIANGLE_H

#include <vector>

#include "nif_math.h"
#include "obj/NiTriBasedGeomData.h"

#include "ProgMesh.h"
#include "CollapseVertex.h"

namespace Niflib {

    class ProgMesh;
    class CollapseVertex;
    class CollapseTriangle;

    class CollapseTriangle {
    public:
        CollapseTriangle(CollapseVertex*, CollapseVertex*, CollapseVertex*);
        ~CollapseTriangle(void);

        unsigned int HasVertex(CollapseVertex*);
        void ReplaceVertex(CollapseVertex*, CollapseVertex*);

        void ComputeNormal(void);

        CollapseVertex* vertex[3];
        Vector3 normal;
    };

} // namespace Niflib

#endif
