
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

    // Simple row-oriented algorithm with accumulator
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            // Initialize accumulator
            SimpleMatrix::value_type sum{0};
            
            // Compute dot product with fixed stride
            constexpr int STRIDE = 4;
            int k = 0;
            
            // Main loop with fixed stride
            for (; k + STRIDE <= K; k += STRIDE) {
                sum += lhs(i,k) * rhs(k,j)
                    + lhs(i,k+1) * rhs(k+1,j)
                    + lhs(i,k+2) * rhs(k+2,j)
                    + lhs(i,k+3) * rhs(k+3,j);
            }
            
            // Handle remaining elements
            for (; k < K; ++k) {
                sum += lhs(i,k) * rhs(k,j);
            }
            
            res(i,j) = sum;
        }
    }
    
    return res;
}
