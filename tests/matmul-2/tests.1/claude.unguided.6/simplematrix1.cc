
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) 
{
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    // Pre-allocate result matrix (assumed to be zero-initialized)
    SimpleMatrix res{M, N};
    
    // Block size for cache optimization
    constexpr int BLOCK_SIZE = 32;  // Adjust based on cache size
    
    // Block multiplication
    for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
        const int imax = std::min(i0 + BLOCK_SIZE, M);
        for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
            const int jmax = std::min(j0 + BLOCK_SIZE, N);
            for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
                const int kmax = std::min(k0 + BLOCK_SIZE, K);
                
                // Multiply blocks
                for (int i = i0; i < imax; ++i) {
                    for (int k = k0; k < kmax; ++k) {
                        const double lhs_ik = lhs(i, k);  // Cache this value
                        for (int j = j0; j < jmax; ++j) {
                            res(i,j) += lhs_ik * rhs(k, j);
                        }
                    }
                }
            }
        }
    }
    
    return res;
}
