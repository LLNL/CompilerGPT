
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            SimpleMatrix::value_type sum = 0;
            
            // Unroll the loop by factor of 4
            int k = 0;
            for (; k + 3 < K; k += 4) {
                // Load values from lhs once per iteration
                SimpleMatrix::value_type lhs_k0 = lhs(i, k);
                SimpleMatrix::value_type lhs_k1 = lhs(i, k+1);
                SimpleMatrix::value_type lhs_k2 = lhs(i, k+2);
                SimpleMatrix::value_type lhs_k3 = lhs(i, k+3);
                
                // Load values from rhs once per iteration
                SimpleMatrix::value_type rhs_k0 = rhs(k, j);
                SimpleMatrix::value_type rhs_k1 = rhs(k+1, j);
                SimpleMatrix::value_type rhs_k2 = rhs(k+2, j);
                SimpleMatrix::value_type rhs_k3 = rhs(k+3, j);
                
                // Perform multiplication and accumulation
                sum += lhs_k0 * rhs_k0 +
                      lhs_k1 * rhs_k1 +
                      lhs_k2 * rhs_k2 +
                      lhs_k3 * rhs_k3;
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
