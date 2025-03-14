
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Cache blocking
    constexpr int BLOCK_SIZE = 32;

    // Initialize result matrix to zero first
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            res(i,j) = SimpleMatrix::value_type{0};
        }
    }

    // Blocked matrix multiplication for better cache usage
    for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
        const int imax = std::min(i0 + BLOCK_SIZE, M);
        for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
            const int kmax = std::min(k0 + BLOCK_SIZE, K);
            for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
                const int jmax = std::min(j0 + BLOCK_SIZE, N);

                for (int i = i0; i < imax; ++i) {
                    for (int j = j0; j < jmax; ++j) {
                        SimpleMatrix::value_type sum = res(i,j);
                        for (int k = k0; k < kmax; ++k) {
                            sum += lhs(i,k) * rhs(k,j);
                        }
                        res(i,j) = sum;
                    }
                }
            }
        }
    }

    return res;
}
