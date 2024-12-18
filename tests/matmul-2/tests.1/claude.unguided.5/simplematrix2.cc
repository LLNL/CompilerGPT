
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Cache line optimization - process in blocks
    constexpr int BLOCK_SIZE = 32;

    for (int ii = 0; ii < M; ii += BLOCK_SIZE) {
        const int i_end = std::min(ii + BLOCK_SIZE, M);
        for (int jj = 0; jj < N; jj += BLOCK_SIZE) {
            const int j_end = std::min(jj + BLOCK_SIZE, N);
            for (int i = ii; i < i_end; ++i) {
                for (int j = jj; j < j_end; ++j) {
                    SimpleMatrix::value_type sum = 0;
                    for (int k = 0; k < K; ++k) {
                        sum += lhs(i,k) * rhs(k,j);
                    }
                    res(i,j) = sum;
                }
            }
        }
    }

    return res;
}
