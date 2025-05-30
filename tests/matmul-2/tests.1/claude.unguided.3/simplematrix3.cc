
#include <stdexcept>
#include "simplematrix.h"

namespace {
    // Helper function for small matrix multiplication
    void multiply_small(const SimpleMatrix& lhs, const SimpleMatrix& rhs, 
                       SimpleMatrix& res, int M, int N, int K) {
        using value_type = SimpleMatrix::value_type;
        
        for (int i = 0; i < M; i += 2) {
            for (int j = 0; j < N; j += 2) {
                value_type sum00 = 0, sum01 = 0;
                value_type sum10 = 0, sum11 = 0;
                
                for (int k = 0; k < K; ++k) {
                    const value_type lhs0k = lhs(i, k);
                    const value_type lhs1k = (i + 1 < M) ? lhs(i + 1, k) : 0;
                    const value_type rhskj0 = rhs(k, j);
                    const value_type rhskj1 = (j + 1 < N) ? rhs(k, j + 1) : 0;
                    
                    sum00 += lhs0k * rhskj0;
                    sum01 += lhs0k * rhskj1;
                    sum10 += lhs1k * rhskj0;
                    sum11 += lhs1k * rhskj1;
                }
                
                res(i, j) = sum00;
                if (j + 1 < N) res(i, j + 1) = sum01;
                if (i + 1 < M) {
                    res(i + 1, j) = sum10;
                    if (j + 1 < N) res(i + 1, j + 1) = sum11;
                }
            }
        }
    }
}

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Small matrix optimization
    if (M <= 32 && N <= 32 && K <= 32) {
        multiply_small(lhs, rhs, res, M, N, K);
        return res;
    }

    // Choose block size based on cache size
    constexpr int BLOCK_SIZE = 32;
    constexpr int MINI_BLOCK_SIZE = 4;

    // Initialize result matrix to zero
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            res(i,j) = 0;
        }
    }

    using value_type = SimpleMatrix::value_type;

    // Block matrix multiplication with mini-blocks
    for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
        for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
            for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
                // Block boundaries
                const int i_end = std::min(i0 + BLOCK_SIZE, M);
                const int j_end = std::min(j0 + BLOCK_SIZE, N);
                const int k_end = std::min(k0 + BLOCK_SIZE, K);

                // Mini-block multiplication
                for (int i = i0; i < i_end; i += MINI_BLOCK_SIZE) {
                    for (int k = k0; k < k_end; k += MINI_BLOCK_SIZE) {
                        for (int j = j0; j < j_end; j += MINI_BLOCK_SIZE) {
                            // Process mini-block
                            for (int ii = i; ii < std::min(i + MINI_BLOCK_SIZE, i_end); ++ii) {
                                for (int kk = k; kk < std::min(k + MINI_BLOCK_SIZE, k_end); ++kk) {
                                    const value_type lhs_val = lhs(ii, kk);
                                    for (int jj = j; jj < std::min(j + MINI_BLOCK_SIZE, j_end); ++jj) {
                                        res(ii,jj) += lhs_val * rhs(kk,jj);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return res;
}
