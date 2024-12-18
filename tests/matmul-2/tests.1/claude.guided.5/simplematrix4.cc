
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Unroll factor chosen to balance register pressure and vectorization
    constexpr int UNROLL = 4;

    // Main computation with manual unrolling to encourage vectorization
    for (int i = 0; i < M; ++i) {
        // Zero initialization combined with computation
        int j;
        for (j = 0; j + UNROLL <= N; j += UNROLL) {
            SimpleMatrix::value_type sum0 = SimpleMatrix::value_type{0};
            SimpleMatrix::value_type sum1 = SimpleMatrix::value_type{0};
            SimpleMatrix::value_type sum2 = SimpleMatrix::value_type{0};
            SimpleMatrix::value_type sum3 = SimpleMatrix::value_type{0};

            for (int k = 0; k < K; ++k) {
                const auto lhs_ik = lhs(i,k);
                sum0 += lhs_ik * rhs(k,j);
                sum1 += lhs_ik * rhs(k,j+1);
                sum2 += lhs_ik * rhs(k,j+2);
                sum3 += lhs_ik * rhs(k,j+3);
            }

            res(i,j)   = sum0;
            res(i,j+1) = sum1;
            res(i,j+2) = sum2;
            res(i,j+3) = sum3;
        }

        // Handle remaining elements
        for (; j < N; ++j) {
            SimpleMatrix::value_type sum = SimpleMatrix::value_type{0};
            for (int k = 0; k < K; ++k) {
                sum += lhs(i,k) * rhs(k,j);
            }
            res(i,j) = sum;
        }
    }

    return res;
}
