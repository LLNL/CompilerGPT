
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) 
{
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};
    using value_type = SimpleMatrix::value_type;

    // Handle small matrices differently for better efficiency
    if (M <= 8 && N <= 8) {
        // Completely unrolled small matrix multiplication
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                value_type sum{0};
                // Manual unrolling for small K
                int k = 0;
                for (; k + 4 <= K; k += 4) {
                    sum += lhs(i, k) * rhs(k, j) +
                          lhs(i, k+1) * rhs(k+1, j) +
                          lhs(i, k+2) * rhs(k+2, j) +
                          lhs(i, k+3) * rhs(k+3, j);
                }
                // Handle remaining elements
                for (; k < K; ++k) {
                    sum += lhs(i, k) * rhs(k, j);
                }
                res(i, j) = sum;
            }
        }
        return res;
    }

    // For larger matrices, use cache-blocked algorithm
    constexpr int BLOCK_SIZE = 32;
    constexpr int MINI_BLOCK = 4;

    // Pre-initialize result matrix to zero
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            res(i, j) = value_type{0};
        }
    }

    // Main blocked multiplication
    for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
        const int imax = std::min(i0 + BLOCK_SIZE, M);
        
        for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
            const int kmax = std::min(k0 + BLOCK_SIZE, K);
            
            for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
                const int jmax = std::min(j0 + BLOCK_SIZE, N);
                
                // Mini-blocked multiplication
                for (int i = i0; i < imax; i += MINI_BLOCK) {
                    const int iimax = std::min(i + MINI_BLOCK, imax);
                    
                    for (int k = k0; k < kmax; k += MINI_BLOCK) {
                        const int kkmax = std::min(k + MINI_BLOCK, kmax);
                        
                        // Load and cache lhs block
                        value_type lhs_cache[MINI_BLOCK][MINI_BLOCK];
                        for (int ii = i; ii < iimax; ++ii) {
                            for (int kk = k; kk < kkmax; ++kk) {
                                lhs_cache[ii-i][kk-k] = lhs(ii, kk);
                            }
                        }
                        
                        for (int j = j0; j < jmax; j += MINI_BLOCK) {
                            const int jjmax = std::min(j + MINI_BLOCK, jmax);
                            
                            // Process mini block
                            for (int ii = i; ii < iimax; ++ii) {
                                for (int jj = j; jj < jjmax; ++jj) {
                                    value_type sum{0};
                                    for (int kk = k; kk < kkmax; ++kk) {
                                        sum += lhs_cache[ii-i][kk-k] * rhs(kk, jj);
                                    }
                                    res(ii, jj) += sum;
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
