#ifndef HEIGHTFIELDSAMPLER_H
#define HEIGHTFIELDSAMPLER_H

#include "nif_math.h"

class HeightFieldSampler {
public:
    HeightFieldSampler(float* d, size_t dh, size_t dw, float t, float l, float b, float r);
    ~HeightFieldSampler();

    Niflib::TexCoord SampleTexCoord(float x, float y);

    float SampleHeight(float x, float y);
    float GetHeightValue(size_t x, size_t y);

private:
    float top, left, bottom, right;
    float* data;
    size_t data_height;
    size_t data_width;
};

#endif
