
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

    // Fixed-size inner block for better register usage
    constexpr int BLOCK = 4;

    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            // Initialize accumulator
            SimpleMatrix::value_type sum{0};
            
            // Process in small blocks to help register allocation
            int k = 0;
            for (; k + BLOCK <= K; k += BLOCK) {
                // Load block of values from lhs once
                const SimpleMatrix::value_type lhs0 = lhs(i,k);
                const SimpleMatrix::value_type lhs1 = lhs(i,k+1);
                const SimpleMatrix::value_type lhs2 = lhs(i,k+2);
                const SimpleMatrix::value_type lhs3 = lhs(i,k+3);
                
                // Load block of values from rhs once
                const SimpleMatrix::value_type rhs0 = rhs(k,j);
                const SimpleMatrix::value_type rhs1 = rhs(k+1,j);
                const SimpleMatrix::value_type rhs2 = rhs(k+2,j);
                const SimpleMatrix::value_type rhs3 = rhs(k+3,j);
                
                // Accumulate products
                sum += lhs0 * rhs0;
                sum += lhs1 * rhs1;
                sum += lhs2 * rhs2;
                sum += lhs3 * rhs3;
            }
            
            // Handle remaining elements
            for (; k < K; ++k) {
                const SimpleMatrix::value_type lhs_val = lhs(i,k);
                const SimpleMatrix::value_type rhs_val = rhs(k,j);
                sum += lhs_val * rhs_val;
            }
            
            res(i,j) = sum;
        }
    }
    
    return res;
}
