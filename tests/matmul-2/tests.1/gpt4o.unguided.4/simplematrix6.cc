
#include <stdexcept>
#include <algorithm> // for std::min
#include "simplematrix.h"

// Define a block size for cache optimization; tuned for specific hardware
constexpr int BLOCK_SIZE = 64; 
constexpr int UNROLL_SIZE = 8;

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows()) {
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};
    }

    SimpleMatrix res(lhs.rows(), rhs.columns());

    const int lhsRows = lhs.rows();
    const int lhsCols = lhs.columns();
    const int rhsCols = rhs.columns();

    // Blocked matrix multiplication with enhanced optimizations
    for (int ii = 0; ii < lhsRows; ii += BLOCK_SIZE) {
        for (int jj = 0; jj < rhsCols; jj += BLOCK_SIZE) {
            for (int kk = 0; kk < lhsCols; kk += BLOCK_SIZE) {

                for (int i = ii; i < std::min(ii + BLOCK_SIZE, lhsRows); ++i) {
                    for (int k = kk; k < std::min(kk + BLOCK_SIZE, lhsCols); ++k) {
                        auto lhsValue = lhs(i, k);

                        int j = jj;
                        for (; j <= std::min(jj + BLOCK_SIZE, rhsCols) - UNROLL_SIZE; j += UNROLL_SIZE) {
                            res(i, j)     += lhsValue * rhs(k, j);
                            res(i, j + 1) += lhsValue * rhs(k, j + 1);
                            res(i, j + 2) += lhsValue * rhs(k, j + 2);
                            res(i, j + 3) += lhsValue * rhs(k, j + 3);
                            res(i, j + 4) += lhsValue * rhs(k, j + 4);
                            res(i, j + 5) += lhsValue * rhs(k, j + 5);
                            res(i, j + 6) += lhsValue * rhs(k, j + 6);
                            res(i, j + 7) += lhsValue * rhs(k, j + 7);
                        }

                        for (; j < std::min(jj + BLOCK_SIZE, rhsCols); ++j) {
                            res(i, j) += lhsValue * rhs(k, j);
                        }
                    }
                }
            }
        }
    }

    return res;
}
