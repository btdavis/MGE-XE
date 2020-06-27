#include "StdAfx.h"
#include "LargeTriangle.h"

LargeTriangle::LargeTriangle() {}

LargeTriangle::LargeTriangle(unsigned int v1, unsigned int v2, unsigned int v3) {
    this->v1 = v1;
    this->v2 = v2;
    this->v3 = v3;
}

void LargeTriangle::Set(unsigned int v1, unsigned int v2, unsigned int v3) {
    this->v1 = v1;
    this->v2 = v2;
    this->v3 = v3;
}

unsigned int& LargeTriangle::operator[](int n) {
    switch (n) {
    case 0:
        return v1;
        break;
    case 1:
        return v2;
        break;
    case 2:
        return v3;
        break;
    default:
        throw std::out_of_range("Index out of range for Triangle");
    };
}
unsigned int LargeTriangle::operator[](int n) const {
    switch (n) {
    case 0:
        return v1;
        break;
    case 1:
        return v2;
        break;
    case 2:
        return v3;
        break;
    default:
        throw std::out_of_range("Index out of range for Triangle");
    };
}
