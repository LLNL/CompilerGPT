
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

    // Initialize result to zero using larger chunks
    for (int i = 0; i < M; i++) {
        SimpleMatrix::value_type* const row = &res(i,0);
        for (int j = 0; j < N; j++) {
            row[j] = 0;
        }
    }

    // Process main body of the matrix
    for (int i = 0; i < M; i++) {
        const int klim = K - (K % 8);  // Ensure K-loop can be fully unrolled
        
        for (int j = 0; j < N; j++) {
            SimpleMatrix::value_type sum = 0;
            SimpleMatrix::value_type sum2 = 0;
            SimpleMatrix::value_type sum3 = 0;
            SimpleMatrix::value_type sum4 = 0;

            // Manually vectorized K-loop with multiple accumulators
            int k = 0;
            for (; k < klim; k += 8) {
                // Load lhs values
                const SimpleMatrix::value_type a0 = lhs(i, k);
                const SimpleMatrix::value_type a1 = lhs(i, k+1);
                const SimpleMatrix::value_type a2 = lhs(i, k+2);
                const SimpleMatrix::value_type a3 = lhs(i, k+3);
                const SimpleMatrix::value_type a4 = lhs(i, k+4);
                const SimpleMatrix::value_type a5 = lhs(i, k+5);
                const SimpleMatrix::value_type a6 = lhs(i, k+6);
                const SimpleMatrix::value_type a7 = lhs(i, k+7);

                // Load rhs values
                const SimpleMatrix::value_type b0 = rhs(k, j);
                const SimpleMatrix::value_type b1 = rhs(k+1, j);
                const SimpleMatrix::value_type b2 = rhs(k+2, j);
                const SimpleMatrix::value_type b3 = rhs(k+3, j);
                const SimpleMatrix::value_type b4 = rhs(k+4, j);
                const SimpleMatrix::value_type b5 = rhs(k+5, j);
                const SimpleMatrix::value_type b6 = rhs(k+6, j);
                const SimpleMatrix::value_type b7 = rhs(k+7, j);

                // Multiply-add using multiple accumulators
                sum += a0 * b0 + a1 * b1;
                sum2 += a2 * b2 + a3 * b3;
                sum3 += a4 * b4 + a5 * b5;
                sum4 += a6 * b6 + a7 * b7;
            }

            // Handle remaining elements
            for (; k < K; k++) {
                sum += lhs(i, k) * rhs(k, j);
            }

            // Combine all accumulators
            res(i, j) = sum + sum2 + sum3 + sum4;
        }
    }

    return res;
}
