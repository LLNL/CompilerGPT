
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

    // Process two rows at once to improve register reuse
    for (int i = 0; i < M - 1; i += 2) {
        for (int j = 0; j < N; ++j) {
            SimpleMatrix::value_type sum1{0};
            SimpleMatrix::value_type sum2{0};
            
            // Main loop with manual vectorization hint
            #pragma clang loop vectorize(enable)
            for (int k = 0; k < K; ++k) {
                // Process two rows simultaneously
                sum1 += lhs(i,k) * rhs(k,j);
                sum2 += lhs(i+1,k) * rhs(k,j);
            }
            
            res(i,j) = sum1;
            res(i+1,j) = sum2;
        }
    }

    // Handle last row if M is odd
    if (M % 2) {
        const int i = M - 1;
        for (int j = 0; j < N; ++j) {
            SimpleMatrix::value_type sum{0};
            
            #pragma clang loop vectorize(enable)
            for (int k = 0; k < K; ++k) {
                sum += lhs(i,k) * rhs(k,j);
            }
            
            res(i,j) = sum;
        }
    }
    
    return res;
}
