
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int m = lhs.rows();
    const int n = rhs.columns();
    const int p = lhs.columns();
    
    SimpleMatrix res{m, n};

    // Smaller blocks for L1 cache
    constexpr int BLOCK_SIZE = 4;

    // Separate initialization to help vectorization
    #pragma clang loop vectorize(enable) interleave(enable)
    for (int i = 0; i < m; ++i) {
        #pragma clang loop vectorize(enable) interleave(enable)
        for (int j = 0; j < n; ++j) {
            res(i,j) = SimpleMatrix::value_type{0};
        }
    }

    // Main computation with focus on vectorization and register pressure
    for (int ii = 0; ii < m; ii += BLOCK_SIZE) {
        const int i_end = std::min(ii + BLOCK_SIZE, m);
        
        for (int kk = 0; kk < p; kk += BLOCK_SIZE) {
            const int k_end = std::min(kk + BLOCK_SIZE, p);
            
            for (int jj = 0; jj < n; jj += BLOCK_SIZE) {
                const int j_end = std::min(jj + BLOCK_SIZE, n);
                
                // Process small blocks for better cache utilization
                for (int i = ii; i < i_end; ++i) {
                    // Prefetch next row of lhs
                    #pragma clang loop vectorize(enable) interleave(enable)
                    for (int k = kk; k < k_end; ++k) {
                        const auto lhs_ik = lhs(i,k);
                        #pragma clang loop vectorize(enable) interleave(enable)
                        for (int j = jj; j < j_end; ++j) {
                            res(i,j) += lhs_ik * rhs(k,j);
                        }
                    }
                }
            }
        }
    }
    
    return res;
}
