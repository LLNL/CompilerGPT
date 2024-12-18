
#include <stdexcept>
#include "simplematrix.h"

constexpr int BLOCK_SIZE = 64; // This can be adapted based on specific cache properties

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    using ValueType = SimpleMatrix::value_type;

    int lhsRows = lhs.rows();
    int lhsCols = lhs.columns();
    int rhsCols = rhs.columns();
    
    if (lhsCols != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    SimpleMatrix res{lhsRows, rhsCols};

    // Precompute the transposed version of rhs for optimal access patterns
    SimpleMatrix rhsTransposed{rhsCols, lhsCols};
    for (int i = 0; i < lhsCols; ++i) {
        for (int j = 0; j < rhsCols; ++j) {
            rhsTransposed(j, i) = rhs(i, j);
        }
    }

    for (int bi = 0; bi < lhsRows; bi += BLOCK_SIZE) {
        for (int bj = 0; bj < rhsCols; bj += BLOCK_SIZE) {
            for (int bk = 0; bk < lhsCols; bk += BLOCK_SIZE) {
                int iEnd = std::min(bi + BLOCK_SIZE, lhsRows);
                int jEnd = std::min(bj + BLOCK_SIZE, rhsCols);
                int kEnd = std::min(bk + BLOCK_SIZE, lhsCols);

                for (int i = bi; i < iEnd; ++i) {
                    for (int j = bj; j < jEnd; ++j) {
                        ValueType sum = res(i, j);
                        int k = bk;

                        // Unroll for further efficiency gain
                        for (; k + 8 <= kEnd; k += 8) {
                            sum += lhs(i, k) * rhsTransposed(j, k);
                            sum += lhs(i, k + 1) * rhsTransposed(j, k + 1);
                            sum += lhs(i, k + 2) * rhsTransposed(j, k + 2);
                            sum += lhs(i, k + 3) * rhsTransposed(j, k + 3);
                            sum += lhs(i, k + 4) * rhsTransposed(j, k + 4);
                            sum += lhs(i, k + 5) * rhsTransposed(j, k + 5);
                            sum += lhs(i, k + 6) * rhsTransposed(j, k + 6);
                            sum += lhs(i, k + 7) * rhsTransposed(j, k + 7);
                        }
                        for (; k < kEnd; ++k) {
                            sum += lhs(i, k) * rhsTransposed(j, k);
                        }

                        res(i, j) = sum;
                    }
                }
            }
        }
    }

    return res;
}
