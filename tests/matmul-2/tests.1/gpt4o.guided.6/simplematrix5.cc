
#include <stdexcept>
#include <vector>
#include <algorithm> // For std::min
#include "simplematrix.h"

using ValueType = SimpleMatrix::value_type;

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    int lhsRows = lhs.rows();
    int lhsCols = lhs.columns();
    int rhsCols = rhs.columns();

    if (lhsCols != rhs.rows()) {
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};
    }

    SimpleMatrix res{lhsRows, rhsCols};

    // Transpose rhs to enhance cache locality
    std::vector<std::vector<ValueType>> rhsT(rhsCols, std::vector<ValueType>(lhsCols));
    for (int i = 0; i < lhsCols; ++i) {
        for (int j = 0; j < rhsCols; ++j) {
            rhsT[j][i] = rhs(i, j);
        }
    }
    
    // Optimize for cache with block-size that suits the architecture (small enough for L1 cache usage)
    const int blockSize = 32; 

    // Blocked matrix multiplication
    for (int ii = 0; ii < lhsRows; ii += blockSize) {
        for (int jj = 0; jj < rhsCols; jj += blockSize) {
            for (int kk = 0; kk < lhsCols; kk += blockSize) {
                
                int iMax = std::min(ii + blockSize, lhsRows);
                int jMax = std::min(jj + blockSize, rhsCols);
                int kMax = std::min(kk + blockSize, lhsCols);

                for (int i = ii; i < iMax; ++i) {
                    for (int j = jj; j < jMax; ++j) {
                        ValueType sum = res(i, j); 

                        for (int k = kk; k < kMax; ++k) {
                            sum += lhs(i, k) * rhsT[j][k];
                        }
                        res(i, j) = sum; // Update the matrix cell with accumulated result
                    }
                }
            }
        }
    }

    return res;
}
