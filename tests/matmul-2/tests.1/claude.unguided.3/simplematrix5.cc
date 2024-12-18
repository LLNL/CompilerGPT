
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    using value_type = SimpleMatrix::value_type;
    constexpr int BLOCK_SIZE = 32;

    // Initialize result matrix
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            res(i,j) = 0;
        }
    }

    // Block matrix multiplication
    for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
        const int iLimit = std::min(i0 + BLOCK_SIZE, M);
        
        for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
            const int kLimit = std::min(k0 + BLOCK_SIZE, K);
            
            for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
                const int jLimit = std::min(j0 + BLOCK_SIZE, N);
                
                // Process block
                for (int i = i0; i < iLimit; ++i) {
                    for (int k = k0; k < kLimit; ++k) {
                        const value_type lhs_val = lhs(i, k);
                        
                        // Inner loop with accumulation
                        for (int j = j0; j < jLimit; ++j) {
                            res(i,j) += lhs_val * rhs(k,j);
                        }
                    }
                }
            }
        }
    }

    return res;
}
