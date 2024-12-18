
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
    using value_type = SimpleMatrix::value_type;
    
    // Optimize for small matrices
    if (M <= 16 && N <= 16 && K <= 16) {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                value_type sum{0};
                for (int k = 0; k < K; ++k) {
                    sum += lhs(i, k) * rhs(k, j);
                }
                res(i, j) = sum;
            }
        }
        return res;
    }

    // For larger matrices use blocked algorithm
    constexpr int BLOCK_SIZE = 32;  
    
    for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
        const int imax = std::min(i0 + BLOCK_SIZE, M);
        
        for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
            const int jmax = std::min(j0 + BLOCK_SIZE, N);
            
            // Initialize result block
            for (int i = i0; i < imax; ++i) {
                for (int j = j0; j < jmax; ++j) {
                    res(i, j) = value_type{0};
                }
            }
            
            for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
                const int kmax = std::min(k0 + BLOCK_SIZE, K);
                
                // Block multiplication with accumulation
                for (int i = i0; i < imax; ++i) {
                    for (int k = k0; k < kmax; ++k) {
                        const value_type lhs_ik = lhs(i, k);
                        for (int j = j0; j < jmax; ++j) {
                            res(i, j) += lhs_ik * rhs(k, j);
                        }
                    }
                }
            }
        }
    }
    
    return res;
}
