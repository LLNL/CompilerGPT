
#include <stdexcept>
#include "simplematrix.h"

namespace {
    // Tuned for modern CPU cache sizes and SIMD width
    constexpr int BLOCK_SIZE = 64;     // L1 cache optimized
    constexpr int MINI_BLOCK = 16;     // Register block size
    constexpr int UNROLL = 4;          // Unroll factor

    // Optimized mini-kernel for 4x4 block multiplication
    inline void multiply_mini_block_4x4(const SimpleMatrix& lhs, const SimpleMatrix& rhs,
                                      SimpleMatrix& res,
                                      int i, int j, int k) {
        // Load and cache lhs values
        const double lhs_0k = lhs(i, k);
        const double lhs_1k = lhs(i+1, k);
        const double lhs_2k = lhs(i+2, k);
        const double lhs_3k = lhs(i+3, k);

        // Load and cache rhs values
        const double rhs_k0 = rhs(k, j);
        const double rhs_k1 = rhs(k, j+1);
        const double rhs_k2 = rhs(k, j+2);
        const double rhs_k3 = rhs(k, j+3);

        // Compute 4x4 block with minimized memory access
        res(i, j)     += lhs_0k * rhs_k0;
        res(i, j+1)   += lhs_0k * rhs_k1;
        res(i, j+2)   += lhs_0k * rhs_k2;
        res(i, j+3)   += lhs_0k * rhs_k3;

        res(i+1, j)   += lhs_1k * rhs_k0;
        res(i+1, j+1) += lhs_1k * rhs_k1;
        res(i+1, j+2) += lhs_1k * rhs_k2;
        res(i+1, j+3) += lhs_1k * rhs_k3;

        res(i+2, j)   += lhs_2k * rhs_k0;
        res(i+2, j+1) += lhs_2k * rhs_k1;
        res(i+2, j+2) += lhs_2k * rhs_k2;
        res(i+2, j+3) += lhs_2k * rhs_k3;

        res(i+3, j)   += lhs_3k * rhs_k0;
        res(i+3, j+1) += lhs_3k * rhs_k1;
        res(i+3, j+2) += lhs_3k * rhs_k2;
        res(i+3, j+3) += lhs_3k * rhs_k3;
    }

    // Process a block with mini-blocks
    inline void multiply_block(const SimpleMatrix& lhs, const SimpleMatrix& rhs,
                             SimpleMatrix& res,
                             int i_start, int i_end,
                             int j_start, int j_end,
                             int k_start, int k_end) {
        for (int i = i_start; i < i_end; i += MINI_BLOCK) {
            const int mini_i_end = std::min(i + MINI_BLOCK, i_end);
            
            for (int j = j_start; j < j_end; j += MINI_BLOCK) {
                const int mini_j_end = std::min(j + MINI_BLOCK, j_end);
                
                // Initialize accumulator block to zero
                for (int ii = i; ii < mini_i_end; ++ii) {
                    for (int jj = j; jj < mini_j_end; ++jj) {
                        res(ii, jj) = 0;
                    }
                }

                // Compute mini-block
                for (int k = k_start; k < k_end; ++k) {
                    for (int ii = i; ii < mini_i_end; ii += UNROLL) {
                        if (ii + UNROLL <= mini_i_end && j + UNROLL <= mini_j_end) {
                            multiply_mini_block_4x4(lhs, rhs, res, ii, j, k);
                        } else {
                            // Handle edge cases
                            for (int iii = ii; iii < std::min(ii + UNROLL, mini_i_end); ++iii) {
                                const double lhs_val = lhs(iii, k);
                                for (int jj = j; jj < mini_j_end; ++jj) {
                                    res(iii, jj) += lhs_val * rhs(k, jj);
                                }
                            }
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

    // Process matrix in blocks
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
