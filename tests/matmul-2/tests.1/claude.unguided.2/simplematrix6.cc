
#include <stdexcept>
#include "simplematrix.h"

namespace {
    constexpr int BLOCK_SIZE = 32;  // Adjust based on cache size
    constexpr int UNROLL = 4;       // Unrolling factor

    // Helper function for block multiplication
    inline void multiply_block(const SimpleMatrix& lhs, const SimpleMatrix& rhs,
                             SimpleMatrix& res,
                             int i_start, int i_end,
                             int j_start, int j_end,
                             int k_start, int k_end) {
        for (int i = i_start; i < i_end; i += UNROLL) {
            if (i + UNROLL <= i_end) {
                for (int k = k_start; k < k_end; ++k) {
                    // Cache values from left matrix
                    const double lhs_0k = lhs(i, k);
                    const double lhs_1k = lhs(i+1, k);
                    const double lhs_2k = lhs(i+2, k);
                    const double lhs_3k = lhs(i+3, k);

                    for (int j = j_start; j < j_end; j += UNROLL) {
                        if (j + UNROLL <= j_end) {
                            // Cache values from right matrix
                            const double rhskj0 = rhs(k, j);
                            const double rhskj1 = rhs(k, j+1);
                            const double rhskj2 = rhs(k, j+2);
                            const double rhskj3 = rhs(k, j+3);

                            // Update result matrix with unrolled computations
                            res(i, j)     += lhs_0k * rhskj0;
                            res(i, j+1)   += lhs_0k * rhskj1;
                            res(i, j+2)   += lhs_0k * rhskj2;
                            res(i, j+3)   += lhs_0k * rhskj3;

                            res(i+1, j)   += lhs_1k * rhskj0;
                            res(i+1, j+1) += lhs_1k * rhskj1;
                            res(i+1, j+2) += lhs_1k * rhskj2;
                            res(i+1, j+3) += lhs_1k * rhskj3;

                            res(i+2, j)   += lhs_2k * rhskj0;
                            res(i+2, j+1) += lhs_2k * rhskj1;
                            res(i+2, j+2) += lhs_2k * rhskj2;
                            res(i+2, j+3) += lhs_2k * rhskj3;

                            res(i+3, j)   += lhs_3k * rhskj0;
                            res(i+3, j+1) += lhs_3k * rhskj1;
                            res(i+3, j+2) += lhs_3k * rhskj2;
                            res(i+3, j+3) += lhs_3k * rhskj3;
                        } else {
                            // Handle remaining columns
                            for (int jj = j; jj < j_end; ++jj) {
                                const double rhskj = rhs(k, jj);
                                res(i, jj)   += lhs_0k * rhskj;
                                res(i+1, jj) += lhs_1k * rhskj;
                                res(i+2, jj) += lhs_2k * rhskj;
                                res(i+3, jj) += lhs_3k * rhskj;
                            }
                        }
                    }
                }
            } else {
                // Handle remaining rows
                for (int ii = i; ii < i_end; ++ii) {
                    for (int k = k_start; k < k_end; ++k) {
                        const double lhs_ik = lhs(ii, k);
                        for (int j = j_start; j < j_end; ++j) {
                            res(ii, j) += lhs_ik * rhs(k, j);
                        }
                    }
                }
            }
        }
    }
}

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) 
{
    const int m = lhs.rows();
    const int n = rhs.columns();
    const int p = lhs.columns();
    
    if (p != rhs.rows())
        throw std::runtime_error{"Matrix dimensions mismatch"};

    SimpleMatrix res{m, n};

    // Initialize result matrix to zero
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            res(i,j) = 0;
        }
    }

    // Block multiplication
    for (int i0 = 0; i0 < m; i0 += BLOCK_SIZE) {
        const int i_end = std::min(i0 + BLOCK_SIZE, m);
        for (int k0 = 0; k0 < p; k0 += BLOCK_SIZE) {
            const int k_end = std::min(k0 + BLOCK_SIZE, p);
            for (int j0 = 0; j0 < n; j0 += BLOCK_SIZE) {
                const int j_end = std::min(j0 + BLOCK_SIZE, n);
                multiply_block(lhs, rhs, res, i0, i_end, j0, j_end, k0, k_end);
            }
        }
    }

    return res;
}
