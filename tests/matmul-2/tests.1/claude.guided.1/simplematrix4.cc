
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Smaller block size to reduce register pressure
    constexpr int BLOCK_SIZE = 16;

    // Process matrices in blocks
    for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
        const int imax = std::min(i0 + BLOCK_SIZE, M);
        for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
            const int jmax = std::min(j0 + BLOCK_SIZE, N);
            
            // Initialize block to zero
            for (int i = i0; i < imax; ++i) {
                for (int j = j0; j < jmax; ++j) {
                    res(i,j) = SimpleMatrix::value_type{0};
                }
            }

            // Compute block
            for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
                const int kmax = std::min(k0 + BLOCK_SIZE, K);
                
                for (int i = i0; i < imax; ++i) {
                    // Cache lhs values for current i
                    SimpleMatrix::value_type lhs_cache[BLOCK_SIZE];
                    for (int k = k0; k < kmax; ++k) {
                        lhs_cache[k - k0] = lhs(i,k);
                    }
                    
                    for (int j = j0; j < jmax; ++j) {
                        SimpleMatrix::value_type sum = res(i,j);
                        // Use cached values
                        for (int k = 0; k < kmax - k0; ++k) {
                            sum += lhs_cache[k] * rhs(k + k0,j);
                        }
                        res(i,j) = sum;
                    }
                }
            }
        }
    }
    return res;
}
