#ifndef SPLITTRIANGLE_H
#define SPLITTRIANGLE_H

#include "nif_math.h"

class SplitTriangle {
public:
    Niflib::Vector3 left;
    Niflib::Vector3 right;
    Niflib::Vector3 top;

    SplitTriangle(const Niflib::Vector3& left, const Niflib::Vector3& right, const Niflib::Vector3& top);
    ~SplitTriangle();

    Niflib::Vector3 GetHypoCenter() const;
    SplitTriangle LeftSplit(const Niflib::Vector3& new_vert) const;
    SplitTriangle RightSplit(const Niflib::Vector3& new_vert) const;
};

#endif
