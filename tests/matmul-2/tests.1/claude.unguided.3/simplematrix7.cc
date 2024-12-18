
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

    // Initialize result matrix
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            res(i,j) = 0;
        }
    }

    constexpr int BLOCK_SIZE = 32;
    constexpr int MINI_BLOCK = 8;
    constexpr int UNROLL = 4;

    // Main blocked matrix multiplication
    for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
        const int iLimit = std::min(i0 + BLOCK_SIZE, M);
        
        for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
            const int kLimit = std::min(k0 + BLOCK_SIZE, K);
            
            for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
                const int jLimit = std::min(j0 + BLOCK_SIZE, N);
                
                // Mini-block processing
                for (int i = i0; i < iLimit; i += MINI_BLOCK) {
                    const int iInnerLimit = std::min(i + MINI_BLOCK, iLimit);
                    
                    for (int k = k0; k < kLimit; ++k) {
                        // Process mini-block rows
                        for (int ii = i; ii < iInnerLimit; ++ii) {
                            const value_type lhs_val = lhs(ii, k);
                            
                            // Unrolled inner loop for better pipelining
                            for (int j = j0; j < jLimit - (UNROLL-1); j += UNROLL) {
                                res(ii,j) += lhs_val * rhs(k,j);
                                res(ii,j+1) += lhs_val * rhs(k,j+1);
                                res(ii,j+2) += lhs_val * rhs(k,j+2);
                                res(ii,j+3) += lhs_val * rhs(k,j+3);
                            }
                            
                            // Handle remaining elements
                            for (int j = jLimit - ((jLimit - j0) % UNROLL); j < jLimit; ++j) {
                                res(ii,j) += lhs_val * rhs(k,j);
                            }
                        }
                    }
                }
            }
        }
    }

    return res;
}
