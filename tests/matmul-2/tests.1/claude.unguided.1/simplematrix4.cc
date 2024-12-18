
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Outer loop blocking for better cache utilization
    constexpr int BLOCK_I = 4;
    constexpr int BLOCK_J = 4;
    
    for (int i0 = 0; i0 < M; i0 += BLOCK_I) {
        const int imax = (i0 + BLOCK_I < M) ? i0 + BLOCK_I : M;
        
        for (int j0 = 0; j0 < N; j0 += BLOCK_J) {
            const int jmax = (j0 + BLOCK_J < N) ? j0 + BLOCK_J : N;
            
            // Pre-initialize block to zero
            for (int i = i0; i < imax; ++i) {
                for (int j = j0; j < jmax; ++j) {
                    res(i, j) = 0;
                }
            }
            
            // Process block with aggressive loop unrolling
            for (int k = 0; k < K; k++) {
                // Cache the k-th column of rhs for the block
                SimpleMatrix::value_type rhs_cache[BLOCK_J];
                for (int j = j0; j < jmax; ++j) {
                    rhs_cache[j - j0] = rhs(k, j);
                }
                
                for (int i = i0; i < imax; ++i) {
                    const SimpleMatrix::value_type lhs_val = lhs(i, k);
                    
                    // Unrolled inner loop
                    int j = j0;
                    for (; j + 3 < jmax; j += 4) {
                        res(i, j) += lhs_val * rhs_cache[j - j0];
                        res(i, j+1) += lhs_val * rhs_cache[j - j0 + 1];
                        res(i, j+2) += lhs_val * rhs_cache[j - j0 + 2];
                        res(i, j+3) += lhs_val * rhs_cache[j - j0 + 3];
                    }
                    
                    // Handle remaining columns
                    for (; j < jmax; ++j) {
                        res(i, j) += lhs_val * rhs_cache[j - j0];
                    }
                }
            }
        }
    }

    return res;
}
