
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Handle each row of the result
    for (int i = 0; i < M; ++i) {
        // Zero initialize current row
        for (int j = 0; j < N; ++j) {
            res(i,j) = SimpleMatrix::value_type{0};
        }

        // Process each column of B one at a time
        for (int k = 0; k < K; ++k) {
            // Load value from A once per k iteration
            const auto lhs_val = lhs(i,k);
            
            // Update entire row of result
            #pragma clang loop vectorize(enable) interleave(enable)
            for (int j = 0; j < N; ++j) {
                res(i,j) += lhs_val * rhs(k,j);
            }
        }
    }

    return res;
}
