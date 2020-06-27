#ifndef LARGETRIANGLE_H
#define LARGETRIANGLE_H

struct LargeTriangle {
    unsigned int v1; /*!< The index of the first vertex. */
    unsigned int v2; /*!< The index of the second vertex. */
    unsigned int v3; /*!< The index of the third vertex. */

    LargeTriangle();
    LargeTriangle(unsigned int v1, unsigned int v2, unsigned int v3);

    void Set(unsigned int v1, unsigned int v2, unsigned int v3);
    unsigned int& operator[](int n);
    unsigned int operator[](int n) const;
};

#endif
