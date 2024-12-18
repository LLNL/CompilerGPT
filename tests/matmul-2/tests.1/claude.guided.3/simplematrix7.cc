
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

    // Alignment for better vectorization
    constexpr int ALIGN = 4;
    const int Npad = (N + ALIGN - 1) & ~(ALIGN - 1);

    // Process matrix row by row
    for (int i = 0; i < M; ++i) {
        // Handle main part of row with aligned access
        int j;
        for (j = 0; j + ALIGN <= N; j += ALIGN) {
            value_type sum0 = value_type{0};
            value_type sum1 = value_type{0};
            value_type sum2 = value_type{0};
            value_type sum3 = value_type{0};

            // Inner product computation
            #pragma clang loop vectorize(assume_safety)
            for (int k = 0; k < K; ++k) {
                const value_type lhs_ik = lhs(i,k);
                sum0 += lhs_ik * rhs(k,j);
                sum1 += lhs_ik * rhs(k,j+1);
                sum2 += lhs_ik * rhs(k,j+2);
                sum3 += lhs_ik * rhs(k,j+3);
            }

            res(i,j) = sum0;
            res(i,j+1) = sum1;
            res(i,j+2) = sum2;
            res(i,j+3) = sum3;
        }

        // Handle remaining elements
        for (; j < N; ++j) {
            value_type sum = value_type{0};
            for (int k = 0; k < K; ++k) {
                sum += lhs(i,k) * rhs(k,j);
            }
            res(i,j) = sum;
        }
    }

    return res;
}
