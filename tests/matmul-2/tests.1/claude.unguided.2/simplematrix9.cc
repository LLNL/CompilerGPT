
#include <stdexcept>
#include "simplematrix.h"

namespace {
    constexpr int BLOCK_SIZE = 32;

    inline void multiply_4x4_block(const SimpleMatrix& lhs, const SimpleMatrix& rhs,
                                 SimpleMatrix& res, int i, int j, int k) {
        using value_type = SimpleMatrix::value_type;  // Use correct type
        
        // Cache lhs values
        const value_type lhs_00 = lhs(i, k);
        const value_type lhs_10 = lhs(i+1, k);
        const value_type lhs_20 = lhs(i+2, k);
        const value_type lhs_30 = lhs(i+3, k);

        // Cache rhs values
        const value_type rhs_00 = rhs(k, j);
        const value_type rhs_01 = rhs(k, j+1);
        const value_type rhs_02 = rhs(k, j+2);
        const value_type rhs_03 = rhs(k, j+3);

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
        using value_type = SimpleMatrix::value_type;  // Use correct type

        // Initialize block to zero
        for (int i = i_start; i < i_end; ++i) {
            for (int j = j_start; j < j_end; ++j) {
                res(i, j) = value_type{0};
            }
        }

        // Process complete 4x4 blocks
        for (int i = i_start; i < i_end - 3; i += 4) {
            for (int j = j_start; j < j_end - 3; j += 4) {
                for (int k = k_start; k < k_end; ++k) {
                    multiply_4x4_block(lhs, rhs, res, i, j, k);
                }
            }
        }

        // Handle remaining elements
        const int i_remain = i_end - ((i_end - i_start) % 4);
        const int j_remain = j_end - ((j_end - j_start) % 4);

        // Handle remaining rows
        for (int i = i_remain; i < i_end; ++i) {
            for (int j = j_start; j < j_end; ++j) {
                value_type sum{0};
                for (int k = k_start; k < k_end; ++k) {
                    sum += lhs(i, k) * rhs(k, j);
                }
                res(i, j) = sum;
            }
        }

        // Handle remaining columns
        for (int i = i_start; i < i_remain; ++i) {
            for (int j = j_remain; j < j_end; ++j) {
                value_type sum{0};
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
