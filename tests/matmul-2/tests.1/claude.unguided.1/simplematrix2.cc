
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Cache frequently used values
    SimpleMatrix::value_type const* const lhs_data = lhs.data();
    SimpleMatrix::value_type const* const rhs_data = rhs.data();
    SimpleMatrix::value_type* const res_data = res.data();

    // Process the matrix multiplication with loop unrolling
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            SimpleMatrix::value_type sum = 0;
            
            // Unroll the loop by factor of 4
            int k = 0;
            for (; k < K - 3; k += 4) {
                sum += lhs(i, k) * rhs(k, j)
                    + lhs(i, k+1) * rhs(k+1, j)
                    + lhs(i, k+2) * rhs(k+2, j)
                    + lhs(i, k+3) * rhs(k+3, j);
            }
            
            // Handle remaining elements
            for (; k < K; ++k) {
                sum += lhs(i, k) * rhs(k, j);
            }
            
            res(i, j) = sum;
        }
    }

    return res;
}
