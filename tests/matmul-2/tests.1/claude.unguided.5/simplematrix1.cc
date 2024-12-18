
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Block size for cache optimization
    constexpr int BLOCK_SIZE = 32;

    // Zero initialization can be done in the constructor
    
    // Block multiplication
    for (int ii = 0; ii < M; ii += BLOCK_SIZE) {
        const int i_end = std::min(ii + BLOCK_SIZE, M);
        for (int jj = 0; jj < N; jj += BLOCK_SIZE) {
            const int j_end = std::min(jj + BLOCK_SIZE, N);
            for (int kk = 0; kk < K; kk += BLOCK_SIZE) {
                const int k_end = std::min(kk + BLOCK_SIZE, K);

                // Process block
                for (int i = ii; i < i_end; ++i) {
                    for (int j = jj; j < j_end; ++j) {
                        double sum = (kk == 0) ? 0.0 : res(i,j);
                        for (int k = kk; k < k_end; ++k) {
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
