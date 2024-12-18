
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

    // Initialize result matrix to zero
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < N; ++j)
            res(i,j) = 0;

    // Block matrix multiplication
    for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
        for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
            for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
                // Block boundaries
                const int i_end = std::min(i0 + BLOCK_SIZE, M);
                const int j_end = std::min(j0 + BLOCK_SIZE, N);
                const int k_end = std::min(k0 + BLOCK_SIZE, K);

                // Process block
                for (int i = i0; i < i_end; ++i) {
                    for (int k = k0; k < k_end; ++k) {
                        const double lhs_val = lhs(i, k);
                        for (int j = j0; j < j_end; ++j) {
                            res(i,j) += lhs_val * rhs(k,j);
                        }
                    }
                }
            }
        }
    }

    return res;
}
