#include "SplitTriangle.h"

using namespace Niflib;

SplitTriangle::SplitTriangle(const Vector3& left, const Vector3& right, const Vector3& top) {
    this->left = left;
    this->right = right;
    this->top = top;
}
SplitTriangle::~SplitTriangle() {}

Vector3 SplitTriangle::GetHypoCenter() const {
    return (right + left) / 2.0f;
}

SplitTriangle SplitTriangle::LeftSplit(const Vector3& new_vert) const {
    return SplitTriangle(right, top, new_vert);
}

SplitTriangle SplitTriangle::RightSplit(const Vector3& new_vert) const {
    return SplitTriangle(top, left, new_vert);
}
