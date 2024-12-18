
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

    // Process the matrix row by row
    for (int i = 0; i < M; ++i) {
        // Process columns in groups of 4
        for (int j = 0; j < N; j += 4) {
            // Initialize accumulators
            value_type sum0 = value_type{0};
            value_type sum1 = value_type{0};
            value_type sum2 = value_type{0};
            value_type sum3 = value_type{0};
            
            // Single pass through K dimension
            for (int k = 0; k < K; ++k) {
                const value_type lhs_ik = lhs(i, k);
                
                // Explicit unrolled operations
                if (j < N) sum0 += lhs_ik * rhs(k, j);
                if (j+1 < N) sum1 += lhs_ik * rhs(k, j+1);
                if (j+2 < N) sum2 += lhs_ik * rhs(k, j+2);
                if (j+3 < N) sum3 += lhs_ik * rhs(k, j+3);
            }
            
            // Store results with bounds checking
            if (j < N) res(i, j) = sum0;
            if (j+1 < N) res(i, j+1) = sum1;
            if (j+2 < N) res(i, j+2) = sum2;
            if (j+3 < N) res(i, j+3) = sum3;
        }
    }

    return res;
}
