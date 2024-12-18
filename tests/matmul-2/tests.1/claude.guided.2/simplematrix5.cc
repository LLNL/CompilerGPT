
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

    // Even smaller blocks to reduce register pressure further
    constexpr int BLOCK_SIZE = 4;

    // Simple vectorizable initialization
    for (int i = 0; i < M * N; ++i) {
        res(i/N, i%N) = 0.0;
    }

    // Main computation with focus on k-loop vectorization
    for (int ii = 0; ii < M; ii += BLOCK_SIZE) {
        const int iEnd = std::min(ii + BLOCK_SIZE, M);
        
        for (int jj = 0; jj < N; jj += BLOCK_SIZE) {
            const int jEnd = std::min(jj + BLOCK_SIZE, N);
            
            // Moved k-loop outside to improve vectorization chances
            for (int k = 0; k < K; ++k) {
                for (int i = ii; i < iEnd; ++i) {
                    const double lhs_ik = lhs(i,k);
                    
                    #pragma clang loop vectorize(enable) interleave(enable)
                    for (int j = jj; j < jEnd; ++j) {
                        res(i,j) += lhs_ik * rhs(k,j);
                    }
                }
            }
        }
    }
    
    return res;
}
