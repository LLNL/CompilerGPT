
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Block size for cache optimization
    constexpr int BLOCK_SIZE = 32;  // Adjust based on cache size

    // Loop tiling/blocking for better cache utilization
    for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
        for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
            for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
                // Block boundaries
                const int i_end = std::min(i0 + BLOCK_SIZE, M);
                const int j_end = std::min(j0 + BLOCK_SIZE, N);
                const int k_end = std::min(k0 + BLOCK_SIZE, K);

                // Process block
                for (int i = i0; i < i_end; ++i) {
                    for (int j = j0; j < j_end; ++j) {
                        double sum = (k0 == 0) ? 0.0 : res(i,j);
                        
                        // Manual loop unrolling for the innermost loop
                        for (int k = k0; k < k_end - 3; k += 4) {
                            sum += lhs(i, k) * rhs(k, j)
                                + lhs(i, k+1) * rhs(k+1, j)
                                + lhs(i, k+2) * rhs(k+2, j)
                                + lhs(i, k+3) * rhs(k+3, j);
                        }
                        
                        // Handle remaining elements
                        for (int k = k_end - (k_end - k0) % 4; k < k_end; ++k) {
                            sum += lhs(i, k) * rhs(k, j);
                        }
                        
                        res(i,j) = sum;
                    }
                }
            }
        }
    }

    return res;
}
