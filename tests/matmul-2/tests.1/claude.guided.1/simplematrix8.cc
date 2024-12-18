
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

    // Process single row at a time with focus on vectorization
    for (int i = 0; i < M; ++i) {
        // Cache entire row of lhs to reduce memory access
        SimpleMatrix::value_type row_cache[K];
        for (int k = 0; k < K; ++k) {
            row_cache[k] = lhs(i,k);
        }

        // Process columns of result
        for (int j = 0; j < N; ++j) {
            SimpleMatrix::value_type sum{0};
            
            // Single vectorizable loop with direct array access
            #pragma clang loop vectorize(enable) interleave(enable)
            for (int k = 0; k < K; ++k) {
                sum += row_cache[k] * rhs(k,j);
            }
            
            res(i,j) = sum;
        }
    }
    
    return res;
}
