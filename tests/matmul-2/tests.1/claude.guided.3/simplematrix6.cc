
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

    // Process matrix row by row
    for (int i = 0; i < M; ++i) {
        // Zero initialize current row
        for (int j = 0; j < N; ++j) {
            res(i,j) = value_type{0};
        }

        // Process K dimension in chunks for better cache usage
        for (int k = 0; k < K; ++k) {
            const value_type lhs_ik = lhs(i,k);
            
            // Process output elements sequentially
            // Keeping the loop simple for vectorization
            for (int j = 0; j < N; ++j) {
                res(i,j) += lhs_ik * rhs(k,j);
            }
        }
    }

    return res;
}
