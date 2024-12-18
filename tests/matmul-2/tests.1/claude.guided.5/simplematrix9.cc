
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Focus on minimal memory operations and clear dependency chains
    for (int i = 0; i < M; ++i) {
        // Zero initialize current row
        for (int j = 0; j < N; ++j) {
            res(i,j) = SimpleMatrix::value_type{0};
        }

        // Process row in two phases to separate memory operations
        for (int k = 0; k < K; ++k) {
            // Phase 1: Load from lhs once
            const auto lhs_val = lhs(i,k);
            
            // Phase 2: Update result row with minimal memory operations
            // Align operations to help compiler recognize patterns
            SimpleMatrix::value_type* const row = &res(i,0);
            const SimpleMatrix::value_type* const rhs_col = &rhs(k,0);
            
            #pragma clang loop vectorize(enable) interleave(enable)
            for (int j = 0; j < N; ++j) {
                row[j] += lhs_val * rhs_col[j];
            }
        }
    }

    return res;
}
