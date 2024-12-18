
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Small block size for better register usage
    constexpr int BLOCK_SIZE = 4;

    // Initialize result matrix
    for (int i = 0; i < M; ++i) {
        #pragma clang loop vectorize(enable) interleave(enable)
        for (int j = 0; j < N; ++j) {
            res(i,j) = 0;
        }
    }

    // Cache blocking with sequential access
    for (int ii = 0; ii < M; ii += BLOCK_SIZE) {
        const int iLimit = std::min(ii + BLOCK_SIZE, M);
        
        for (int kk = 0; kk < K; kk += BLOCK_SIZE) {
            const int kLimit = std::min(kk + BLOCK_SIZE, K);
            
            for (int jj = 0; jj < N; jj += BLOCK_SIZE) {
                const int jLimit = std::min(jj + BLOCK_SIZE, N);

                // Process block by block without temporary arrays
                for (int i = ii; i < iLimit; ++i) {
                    for (int k = kk; k < kLimit; ++k) {
                        const auto lhs_val = lhs(i,k);
                        
                        // Inner loop with better locality
                        #pragma clang loop vectorize(enable) interleave(enable)
                        for (int j = jj; j < jLimit; ++j) {
                            res(i,j) += lhs_val * rhs(k,j);
                        }
                    }
                }
            }
        }
    }
    
    return res;
}
