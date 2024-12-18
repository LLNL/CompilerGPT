
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Simple approach focusing on minimal register pressure
    // and clear vectorization opportunities
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            // Initialize accumulator outside inner loop
            SimpleMatrix::value_type sum = SimpleMatrix::value_type{0};
            
            // Keep k-loop as simple as possible for vectorization
            #pragma clang loop vectorize(enable) interleave(enable)
            for (int k = 0; k < K; ++k) {
                sum += lhs(i,k) * rhs(k,j);
            }
            
            res(i,j) = sum;
        }
    }

    return res;
}
