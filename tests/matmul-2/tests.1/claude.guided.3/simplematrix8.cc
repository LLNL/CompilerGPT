
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

    // Process in 2x2 blocks to reduce register pressure while maintaining locality
    constexpr int BLOCK = 2;
    
    for (int i = 0; i < M; i += BLOCK) {
        for (int j = 0; j < N; j += BLOCK) {
            // Initialize block accumulators
            value_type block[BLOCK][BLOCK] = {{0}};
            
            // Compute 2x2 block
            #pragma clang loop vectorize(enable) interleave(enable)
            for (int k = 0; k < K; ++k) {
                // Load inputs once for block
                const value_type lhs0 = lhs(i, k);
                const value_type lhs1 = (i+1 < M) ? lhs(i+1, k) : 0;
                const value_type rhs0 = rhs(k, j);
                const value_type rhs1 = (j+1 < N) ? rhs(k, j+1) : 0;
                
                // Update block
                block[0][0] += lhs0 * rhs0;
                block[0][1] += lhs0 * rhs1;
                block[1][0] += lhs1 * rhs0;
                block[1][1] += lhs1 * rhs1;
            }
            
            // Store block results
            res(i, j) = block[0][0];
            if (j+1 < N) res(i, j+1) = block[0][1];
            if (i+1 < M) {
                res(i+1, j) = block[1][0];
                if (j+1 < N) res(i+1, j+1) = block[1][1];
            }
        }
    }

    return res;
}
