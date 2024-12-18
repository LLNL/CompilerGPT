
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

    // Reduced block sizes to lower register pressure
    constexpr int BLOCK_SIZE = 8;
    constexpr int MINI_BLOCK = 2;

    // Simple initialization to avoid register pressure
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            res(i,j) = 0.0;
        }
    }

    // Main computation with reduced register pressure
    for (int ii = 0; ii < M; ii += BLOCK_SIZE) {
        const int iEnd = std::min(ii + BLOCK_SIZE, M);
        
        for (int kk = 0; kk < K; kk += BLOCK_SIZE) {
            const int kEnd = std::min(kk + BLOCK_SIZE, K);
            
            for (int i = ii; i < iEnd; ++i) {
                for (int k = kk; k < kEnd; ++k) {
                    const double val = lhs(i,k);
                    
                    // Process two elements at a time to reduce register pressure
                    // while maintaining some instruction-level parallelism
                    int j = 0;
                    for (; j + MINI_BLOCK <= N; j += MINI_BLOCK) {
                        double sum0 = res(i,j);
                        double sum1 = res(i,j+1);
                        
                        sum0 += val * rhs(k,j);
                        sum1 += val * rhs(k,j+1);
                        
                        res(i,j) = sum0;
                        res(i,j+1) = sum1;
                    }
                    
                    // Handle remaining elements
                    for (; j < N; ++j) {
                        res(i,j) += val * rhs(k,j);
                    }
                }
            }
        }
    }
    
    return res;
}
