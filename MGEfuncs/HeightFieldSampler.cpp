#include "HeightFieldSampler.h"

using namespace Niflib;

HeightFieldSampler::HeightFieldSampler(float* d, size_t dh, size_t dw, float t, float l, float b, float r) :
    top(t), left(l), bottom(b), right(r), data(d), data_height(dh), data_width(dw) {}

HeightFieldSampler::~HeightFieldSampler() {}

TexCoord HeightFieldSampler::SampleTexCoord(float x, float y) {
    float tx = (0.0f - left + x) / (right - left);
    float ty = 1.0f - ((0.0f - bottom + y) / (top - bottom));
    return TexCoord(tx, ty);
}

float HeightFieldSampler::SampleHeight(float x, float y) {
    // Figure which height values to sample.
    size_t low_x, high_x, low_y, high_y;

    double data_x = (0.0 - left + x) * (double)data_width / ((double)right - left);
    double data_y = (0.0 - bottom + y) * (double)data_height / ((double)top - bottom);

    low_x = (size_t)floor(data_x);
    high_x = (size_t)ceil(data_x);
    low_y = (size_t)floor(data_y);
    high_y = (size_t)ceil(data_y);

    // Linear Interpolation
    float x_interp = 1.0f;
    if (high_x - low_x == 1) {
        x_interp = data_x - (float)low_x;
    }

    float y_interp = 1.0f;
    if (high_y - low_y == 1) {
        y_interp = data_y - (float)low_y;
    }

    // horizontal
    float bottom_val = GetHeightValue(low_x, low_y) * (1.0f - x_interp) + GetHeightValue(high_x, low_y) * x_interp;
    float top_val = GetHeightValue(low_x, high_y) * (1.0f - x_interp) + GetHeightValue(high_x, high_y) * x_interp;

    // vertical
    float result = top_val * (1.0f - y_interp) + bottom_val * y_interp;
    return result;
}

float HeightFieldSampler::GetHeightValue(size_t x, size_t y) {
    if (x < 0) {
        x = 0;
    }
    if (x > data_width - 1) {
        x = data_width - 1;
    }
    if (y < 0) {
        y = 0;
    }
    if (y > data_height - 1) {
        y = data_height - 1;
    }

    return data[y * data_width + x];
}
