
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    using ValueType = SimpleMatrix::value_type;

    int lhsRows = lhs.rows();
    int lhsCols = lhs.columns();
    int rhsCols = rhs.columns();
    
    if (lhsCols != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    SimpleMatrix res{lhsRows, rhsCols};

    // Transpose rhs for better cache performance
    SimpleMatrix rhsTransposed{rhsCols, lhsCols};
    for (int i = 0; i < lhsCols; ++i) {
        for (int j = 0; j < rhsCols; ++j) {
            rhsTransposed(j, i) = rhs(i, j);
        }
    }

    // Use a small block size to improve cache locality further
    const int blockSize = 64 / sizeof(ValueType); // A typical cache line size
    for (int bi = 0; bi < lhsRows; bi += blockSize) {
        for (int bj = 0; bj < rhsCols; bj += blockSize) {
            for (int i = bi; i < std::min(bi + blockSize, lhsRows); ++i) {
                for (int j = bj; j < std::min(bj + blockSize, rhsCols); ++j) {
                    ValueType sum = res(i, j);
                    int k = 0;
                    
                    // Further unrolling in steps of 4
                    for (; k + 3 < lhsCols; k += 4) {
                        sum += lhs(i, k) * rhsTransposed(j, k);
                        sum += lhs(i, k + 1) * rhsTransposed(j, k + 1);
                        sum += lhs(i, k + 2) * rhsTransposed(j, k + 2);
                        sum += lhs(i, k + 3) * rhsTransposed(j, k + 3);
                    }
                    // Handle remaining elements
                    for (; k < lhsCols; ++k) {
                        sum += lhs(i, k) * rhsTransposed(j, k);
                    }
                    res(i, j) = sum;
                }
            }
        }
    }

    return res;
}
