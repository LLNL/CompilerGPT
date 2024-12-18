
#include <stdexcept>
#include <algorithm> // for std::min and std::max
#include "simplematrix.h"

// Define constants for blocking and unrolling
constexpr int BLOCK_SIZE = 64;
constexpr int UNROLL_SIZE = 4; // Adjusted for balance between footprint and computational effort

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows()) {
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};
    }

    SimpleMatrix res(lhs.rows(), rhs.columns());

    const int lhsRows = lhs.rows();
    const int lhsCols = lhs.columns();
    const int rhsCols = rhs.columns();

    for (int ii = 0; ii < lhsRows; ii += BLOCK_SIZE) {
        for (int jj = 0; jj < rhsCols; jj += BLOCK_SIZE) {
            for (int kk = 0; kk < lhsCols; kk += BLOCK_SIZE) {

                // Iterate over blocks, then rows, columns, and depth for calculations
                for (int i = ii; i < std::min(ii + BLOCK_SIZE, lhsRows); ++i) {
                    for (int k = kk; k < std::min(kk + BLOCK_SIZE, lhsCols); ++k) {
                        auto lhsValue = lhs(i, k);

                        int j = jj;
                        for (; j <= std::min(jj + BLOCK_SIZE, rhsCols) - UNROLL_SIZE; j += UNROLL_SIZE) {
                            // Staggered accumulation for multiple columns per row element
                            res(i, j)     += lhsValue * rhs(k, j);
                            res(i, j + 1) += lhsValue * rhs(k, j + 1);
                            res(i, j + 2) += lhsValue * rhs(k, j + 2);
                            res(i, j + 3) += lhsValue * rhs(k, j + 3);
                        }

                        // Handle remaining columns
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
