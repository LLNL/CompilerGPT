
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Vectorization-friendly chunk size
    constexpr int CHUNK = 4;

    // Process each row
    for (int i = 0; i < M; ++i) {
        // Zero initialize
        for (int j = 0; j < N; ++j) {
            res(i,j) = SimpleMatrix::value_type{0};
        }

        // Process in chunks that fit vector registers
        for (int k = 0; k < K; ++k) {
            const auto lhs_val = lhs(i,k);
            
            // Main chunk processing
            int j = 0;
            for (; j + CHUNK <= N; j += CHUNK) {
                // Force compiler to handle 4 elements at once
                SimpleMatrix::value_type r0 = res(i,j);
                SimpleMatrix::value_type r1 = res(i,j+1);
                SimpleMatrix::value_type r2 = res(i,j+2);
                SimpleMatrix::value_type r3 = res(i,j+3);

                const auto v0 = rhs(k,j);
                const auto v1 = rhs(k,j+1);
                const auto v2 = rhs(k,j+2);
                const auto v3 = rhs(k,j+3);

                r0 += lhs_val * v0;
                r1 += lhs_val * v1;
                r2 += lhs_val * v2;
                r3 += lhs_val * v3;

                res(i,j)   = r0;
                res(i,j+1) = r1;
                res(i,j+2) = r2;
                res(i,j+3) = r3;
            }

            // Handle remaining elements
            for (; j < N; ++j) {
                res(i,j) += lhs_val * rhs(k,j);
            }
        }
    }

    return res;
}
