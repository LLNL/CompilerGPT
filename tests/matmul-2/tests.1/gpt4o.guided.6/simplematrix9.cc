
#include <stdexcept>
#include <vector>
#include <algorithm>
#include "simplematrix.h"

using ValueType = SimpleMatrix::value_type;

// Transpose function that supports inlining
std::vector<std::vector<ValueType>> inlineTransposeMatrix(const SimpleMatrix& matrix) {
    int rows = matrix.rows();
    int cols = matrix.columns();
    std::vector<std::vector<ValueType>> transposed(cols, std::vector<ValueType>(rows));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            transposed[j][i] = matrix(i, j);
        }
    }
    return transposed;
}

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    int lhsRows = lhs.rows();
    int lhsCols = lhs.columns();
    int rhsCols = rhs.columns();

    if (lhsCols != rhs.rows()) {
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};
    }

    SimpleMatrix res(lhsRows, rhsCols);

    // Precompute transposed RHS matrix, which enhances cache performance
    auto rhsT = inlineTransposeMatrix(rhs);

    const int blockSize = 32; // Block size optimized for cache

    for (int ii = 0; ii < lhsRows; ii += blockSize) {
        for (int jj = 0; jj < rhsCols; jj += blockSize) {
            for (int kk = 0; kk < lhsCols; kk += blockSize) {
                
                int iMax = std::min(ii + blockSize, lhsRows);
                int jMax = std::min(jj + blockSize, rhsCols);
                int kMax = std::min(kk + blockSize, lhsCols);

                // Main multiplication loop with attention to direct access
                for (int i = ii; i < iMax; ++i) {
                    for (int j = jj; j < jMax; ++j) {
                        ValueType sum = res(i, j);

                        // Direct access to matrix elements without taking addresses
                        for (int k = kk; k < kMax; ++k) {
                            sum += lhs(i, k) * rhsT[j][k];
                        }
                        res(i, j) = sum;
                    }
                }
            }
        }
    }

    return res;
}
