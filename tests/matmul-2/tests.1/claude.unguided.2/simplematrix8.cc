
#include <stdexcept>
#include "simplematrix.h"

namespace {
    // Optimized for common L1 cache sizes
    constexpr int BLOCK_SIZE = 32;

    inline void multiply_4x4_block(const SimpleMatrix& lhs, const SimpleMatrix& rhs,
                                 SimpleMatrix& res, int i, int j, int k) {
        // Cache lhs values
        const double lhs_00 = lhs(i, k);
        const double lhs_10 = lhs(i+1, k);
        const double lhs_20 = lhs(i+2, k);
        const double lhs_30 = lhs(i+3, k);

        // Cache rhs values
        const double rhs_00 = rhs(k, j);
        const double rhs_01 = rhs(k, j+1);
        const double rhs_02 = rhs(k, j+2);
        const double rhs_03 = rhs(k, j+3);

        // Compute 4x4 block
        res(i, j)     += lhs_00 * rhs_00;
        res(i, j+1)   += lhs_00 * rhs_01;
        res(i, j+2)   += lhs_00 * rhs_02;
        res(i, j+3)   += lhs_00 * rhs_03;

        res(i+1, j)   += lhs_10 * rhs_00;
        res(i+1, j+1) += lhs_10 * rhs_01;
        res(i+1, j+2) += lhs_10 * rhs_02;
        res(i+1, j+3) += lhs_10 * rhs_03;

        res(i+2, j)   += lhs_20 * rhs_00;
        res(i+2, j+1) += lhs_20 * rhs_01;
        res(i+2, j+2) += lhs_20 * rhs_02;
        res(i+2, j+3) += lhs_20 * rhs_03;

        res(i+3, j)   += lhs_30 * rhs_00;
        res(i+3, j+1) += lhs_30 * rhs_01;
        res(i+3, j+2) += lhs_30 * rhs_02;
        res(i+3, j+3) += lhs_30 * rhs_03;
    }

    inline void multiply_block(const SimpleMatrix& lhs, const SimpleMatrix& rhs,
                             SimpleMatrix& res,
                             int i_start, int i_end,
                             int j_start, int j_end,
                             int k_start, int k_end) {
        for (int i = i_start; i < i_end - 3; i += 4) {
            for (int j = j_start; j < j_end - 3; j += 4) {
                // Initialize block to zero
                for (int ii = 0; ii < 4; ++ii) {
                    for (int jj = 0; jj < 4; ++jj) {
                        res(i + ii, j + jj) = 0;
                    }
                }

                // Compute block
                for (int k = k_start; k < k_end; ++k) {
                    multiply_4x4_block(lhs, rhs, res, i, j, k);
                }
            }

            // Handle remaining columns
            for (int j = j_end - (j_end - j_start) % 4; j < j_end; ++j) {
                double sum = 0;
                for (int k = k_start; k < k_end; ++k) {
                    sum += lhs(i, k) * rhs(k, j);
                }
                res(i, j) = sum;
            }
        }

        // Handle remaining rows
        for (int i = i_end - (i_end - i_start) % 4; i < i_end; ++i) {
            for (int j = j_start; j < j_end; ++j) {
                double sum = 0;
                for (int k = k_start; k < k_end; ++k) {
                    sum += lhs(i, k) * rhs(k, j);
                }
                res(i, j) = sum;
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
