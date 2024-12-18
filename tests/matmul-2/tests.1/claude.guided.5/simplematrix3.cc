
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Smaller block size to reduce register pressure
    constexpr int BLOCK_SIZE = 16;

    // Initialize with simple loop to allow vectorization
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            res(i,j) = SimpleMatrix::value_type{0};
        }
    }

    // Main computation with reduced register pressure
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            SimpleMatrix::value_type sum = SimpleMatrix::value_type{0};
            // Keep k-loop as innermost loop for better vectorization
            for (int k = 0; k < K; ++k) {
                sum += lhs(i,k) * rhs(k,j);
            }
            res(i,j) = sum;
        }
    }

    return res;
}
