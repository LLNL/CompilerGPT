
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Process data in smaller chunks to reduce register pressure
    constexpr int TILE = 8;

    for (int i = 0; i < M; ++i) {
        for (int jj = 0; jj < N; jj += TILE) {
            // Pre-initialize tile of result to zero
            const int jend = (jj + TILE < N) ? jj + TILE : N;
            for (int j = jj; j < jend; ++j) {
                res(i,j) = SimpleMatrix::value_type{0};
            }

            // Compute tile
            for (int k = 0; k < K; ++k) {
                // Load once, reuse for tile
                const auto lhs_ik = lhs(i,k);
                
                #pragma clang loop vectorize(enable) interleave(enable)
                for (int j = jj; j < jend; ++j) {
                    res(i,j) += lhs_ik * rhs(k,j);
                }
            }
        }
    }

    return res;
}
