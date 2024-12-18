
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
            res(i,j) = value_type{0};
        }
    }

    // Smaller block size to reduce register pressure
    constexpr int BLOCK_SIZE = 8;

    // Loop restructured for better register utilization
    for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
        const int iLimit = std::min(i0 + BLOCK_SIZE, M);
        
        for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
            const int kLimit = std::min(k0 + BLOCK_SIZE, K);
            
            for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
                const int jLimit = std::min(j0 + BLOCK_SIZE, N);

                for (int i = i0; i < iLimit; ++i) {
                    // Prefetch next row of lhs
                    value_type lhs_cache[BLOCK_SIZE];
                    for (int k = k0; k < kLimit; ++k) {
                        lhs_cache[k - k0] = lhs(i, k);
                    }

                    for (int j = j0; j < jLimit; ++j) {
                        value_type sum = res(i,j);
                        
                        // Use cached values
                        #pragma clang loop vectorize(enable) interleave(enable)
                        for (int k = 0; k < kLimit - k0; ++k) {
                            sum += lhs_cache[k] * rhs(k + k0, j);
                        }
                        
                        res(i,j) = sum;
                    }
                }
            }
        }
    }

    return res;
}
