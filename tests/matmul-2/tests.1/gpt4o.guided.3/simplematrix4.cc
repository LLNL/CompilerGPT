
#include <stdexcept>

#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    using ValueType = SimpleMatrix::value_type;
    
    int lhs_rows = lhs.rows();
    int lhs_columns = lhs.columns();
    int rhs_columns = rhs.columns();

    if (lhs_columns != rhs.rows()) {
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};
    }

    SimpleMatrix res{lhs_rows, rhs_columns};

    // Attempt cache blocking / tiling for better cache usage
    const int blockSize = 16; // Optimal size may depend on cache size and matrix layout

    for (int jj = 0; jj < rhs_columns; jj += blockSize) {
        for (int kk = 0; kk < lhs_columns; kk += blockSize) {
            for (int i = 0; i < lhs_rows; ++i) {
                for (int j = jj; j < std::min(jj + blockSize, rhs_columns); ++j) {
                    ValueType sum = res(i, j);
                    for (int k = kk; k < std::min(kk + blockSize, lhs_columns); ++k) {
                        sum += lhs(i, k) * rhs(k, j);
                    }
                    res(i, j) = sum;
                }
            }
        }
    }

    return res;
}
