
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Loop tiling/blocking for better cache utilization
    constexpr int BLOCK_SIZE = 32;  // Adjust based on cache size
    
    for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
        for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
            for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
                // Block boundaries
                const int iLimit = std::min(i0 + BLOCK_SIZE, M);
                const int jLimit = std::min(j0 + BLOCK_SIZE, N);
                const int kLimit = std::min(k0 + BLOCK_SIZE, K);

                for (int i = i0; i < iLimit; ++i) {
                    for (int j = j0; j < jLimit; ++j) {
                        double sum = (k0 == 0) ? 0.0 : res(i,j);
                        for (int k = k0; k < kLimit; ++k) {
                            sum += lhs(i, k) * rhs(k, j);
                        }
                        res(i,j) = sum;
                    }
                }
            }
        }
    }
    return res;
}
