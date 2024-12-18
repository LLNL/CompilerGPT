
#include <stdexcept>
#include "simplematrix.h"

namespace {
    constexpr int TINY_THRESHOLD = 4;
    constexpr int SMALL_THRESHOLD = 16;
    constexpr int L1_BLOCK = 48;      // Optimized for common L1 cache sizes
    constexpr int MINI_BLOCK = 6;     // For better register utilization
    constexpr int MICRO_BLOCK = 3;    // For vectorization potential

    // Ultra-optimized 4x4 multiplication
    inline void multiply_tiny(const SimpleMatrix& lhs, const SimpleMatrix& rhs, 
                            SimpleMatrix& res, int M, int N, int K) {
        using value_type = SimpleMatrix::value_type;
        value_type acc[TINY_THRESHOLD][TINY_THRESHOLD] = {{}};
        
        for (int k = 0; k < K; ++k) {
            for (int i = 0; i < M; ++i) {
                const value_type lhs_ik = lhs(i,k);
                #pragma unroll
                for (int j = 0; j < N; ++j) {
                    acc[i][j] += lhs_ik * rhs(k,j);
                }
            }
        }
        
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                res(i,j) = acc[i][j];
            }
        }
    }

    // Optimized small matrix multiplication with loop unrolling
    inline void multiply_small(const SimpleMatrix& lhs, const SimpleMatrix& rhs, 
                             SimpleMatrix& res, int M, int N, int K) {
        using value_type = SimpleMatrix::value_type;
        value_type acc[SMALL_THRESHOLD][SMALL_THRESHOLD] = {{}};

        for (int k = 0; k < K; k++) {
            for (int i = 0; i < M; i++) {
                const value_type lhs_ik = lhs(i,k);
                for (int j = 0; j < N; j += MICRO_BLOCK) {
                    const int jend = std::min(j + MICRO_BLOCK, N);
                    #pragma unroll
                    for (int jj = j; jj < jend; ++jj) {
                        acc[i][jj] += lhs_ik * rhs(k,jj);
                    }
                }
            }
        }

        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                res(i,j) = acc[i][j];
            }
        }
    }
}

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) 
{
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};
    using value_type = SimpleMatrix::value_type;

    // Handle tiny matrices
    if (M <= TINY_THRESHOLD && N <= TINY_THRESHOLD && K <= TINY_THRESHOLD) {
        multiply_tiny(lhs, rhs, res, M, N, K);
        return res;
    }

    // Handle small matrices
    if (M <= SMALL_THRESHOLD && N <= SMALL_THRESHOLD) {
        multiply_small(lhs, rhs, res, M, N, K);
        return res;
    }

    // Initialize result matrix
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            res(i,j) = value_type{0};
        }
    }

    // Main computation with three-level blocking
    for (int i0 = 0; i0 < M; i0 += L1_BLOCK) {
        const int imax = std::min(i0 + L1_BLOCK, M);
        
        for (int k0 = 0; k0 < K; k0 += L1_BLOCK) {
            const int kmax = std::min(k0 + L1_BLOCK, K);
            
            // Cache lhs block
            value_type lhs_cache[L1_BLOCK][L1_BLOCK];
            for (int i = i0; i < imax; ++i) {
                for (int k = k0; k < kmax; ++k) {
                    lhs_cache[i-i0][k-k0] = lhs(i,k);
                }
            }
            
            for (int j0 = 0; j0 < N; j0 += L1_BLOCK) {
                const int jmax = std::min(j0 + L1_BLOCK, N);
                
                // Mini-block multiplication
                for (int i = i0; i < imax; i += MINI_BLOCK) {
                    const int iimax = std::min(i + MINI_BLOCK, imax);
                    
                    for (int j = j0; j < jmax; j += MINI_BLOCK) {
                        const int jjmax = std::min(j + MINI_BLOCK, jmax);
                        
                        // Local accumulators
                        value_type local_acc[MINI_BLOCK][MINI_BLOCK] = {{}};
                        
                        // Micro-blocked multiplication
                        for (int k = k0; k < kmax; k += MICRO_BLOCK) {
                            const int kkmax = std::min(k + MICRO_BLOCK, kmax);
                            
                            // Process micro block
                            for (int ii = i; ii < iimax; ++ii) {
                                for (int kk = k; kk < kkmax; ++kk) {
                                    const value_type lhs_val = lhs_cache[ii-i0][kk-k0];
                                    for (int jj = j; jj < jjmax; ++jj) {
                                        local_acc[ii-i][jj-j] += lhs_val * rhs(kk,jj);
                                    }
                                }
                            }
                        }
                        
                        // Update result
                        for (int ii = i; ii < iimax; ++ii) {
                            for (int jj = j; jj < jjmax; ++jj) {
                                res(ii,jj) += local_acc[ii-i][jj-j];
                            }
                        }
                    }
                }
            }
        }
    }
    
    return res;
}
