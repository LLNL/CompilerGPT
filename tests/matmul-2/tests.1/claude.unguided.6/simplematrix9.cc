
#include <stdexcept>
#include "simplematrix.h"

namespace {
    // Configuration constants optimized for modern CPU architectures
    constexpr int TINY_THRESHOLD = 4;
    constexpr int SMALL_THRESHOLD = 16;
    constexpr int L1_BLOCK = 56;      // Optimized for 32KB L1D cache
    constexpr int MINI_BLOCK = 7;     // Optimized for AVX-512 register count
    constexpr int MICRO_BLOCK = 4;    // Aligned with SIMD width
    constexpr int UNROLL = 4;         // Loop unrolling factor

    using value_type = SimpleMatrix::value_type;

    // Specialized 4x4 multiplication with full unrolling
    inline void multiply_4x4(const SimpleMatrix& lhs, const SimpleMatrix& rhs, SimpleMatrix& res) {
        value_type a00 = lhs(0,0), a01 = lhs(0,1), a02 = lhs(0,2), a03 = lhs(0,3);
        value_type a10 = lhs(1,0), a11 = lhs(1,1), a12 = lhs(1,2), a13 = lhs(1,3);
        value_type a20 = lhs(2,0), a21 = lhs(2,1), a22 = lhs(2,2), a23 = lhs(2,3);
        value_type a30 = lhs(3,0), a31 = lhs(3,1), a32 = lhs(3,2), a33 = lhs(3,3);

        value_type b00 = rhs(0,0), b01 = rhs(0,1), b02 = rhs(0,2), b03 = rhs(0,3);
        value_type b10 = rhs(1,0), b11 = rhs(1,1), b12 = rhs(1,2), b13 = rhs(1,3);
        value_type b20 = rhs(2,0), b21 = rhs(2,1), b22 = rhs(2,2), b23 = rhs(2,3);
        value_type b30 = rhs(3,0), b31 = rhs(3,1), b32 = rhs(3,2), b33 = rhs(3,3);

        res(0,0) = a00*b00 + a01*b10 + a02*b20 + a03*b30;
        res(0,1) = a00*b01 + a01*b11 + a02*b21 + a03*b31;
        res(0,2) = a00*b02 + a01*b12 + a02*b22 + a03*b32;
        res(0,3) = a00*b03 + a01*b13 + a02*b23 + a03*b33;

        res(1,0) = a10*b00 + a11*b10 + a12*b20 + a13*b30;
        res(1,1) = a10*b01 + a11*b11 + a12*b21 + a13*b31;
        res(1,2) = a10*b02 + a11*b12 + a12*b22 + a13*b32;
        res(1,3) = a10*b03 + a11*b13 + a12*b23 + a13*b33;

        res(2,0) = a20*b00 + a21*b10 + a22*b20 + a23*b30;
        res(2,1) = a20*b01 + a21*b11 + a22*b21 + a23*b31;
        res(2,2) = a20*b02 + a21*b12 + a22*b22 + a23*b32;
        res(2,3) = a20*b03 + a21*b13 + a22*b23 + a23*b33;

        res(3,0) = a30*b00 + a31*b10 + a32*b20 + a33*b30;
        res(3,1) = a30*b01 + a31*b11 + a32*b21 + a33*b31;
        res(3,2) = a30*b02 + a31*b12 + a32*b22 + a33*b32;
        res(3,3) = a30*b03 + a31*b13 + a32*b23 + a33*b33;
    }

    // Optimized small matrix multiplication
    inline void multiply_small(const SimpleMatrix& lhs, const SimpleMatrix& rhs, 
                             SimpleMatrix& res, int M, int N, int K) {
        alignas(64) value_type acc[SMALL_THRESHOLD][SMALL_THRESHOLD] = {{}};

        for (int k = 0; k < K; k++) {
            for (int i = 0; i < M; i++) {
                const value_type lhs_ik = lhs(i,k);
                for (int j = 0; j < N; j += UNROLL) {
                    if (j + UNROLL <= N) {
                        acc[i][j] += lhs_ik * rhs(k,j);
                        acc[i][j+1] += lhs_ik * rhs(k,j+1);
                        acc[i][j+2] += lhs_ik * rhs(k,j+2);
                        acc[i][j+3] += lhs_ik * rhs(k,j+3);
                    } else {
                        for (int jj = j; jj < N; ++jj) {
                            acc[i][jj] += lhs_ik * rhs(k,jj);
                        }
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

    // Specialized paths for different matrix sizes
    if (M == 4 && N == 4 && K == 4) {
        multiply_4x4(lhs, rhs, res);
        return res;
    }

    if (M <= SMALL_THRESHOLD && N <= SMALL_THRESHOLD) {
        multiply_small(lhs, rhs, res, M, N, K);
        return res;
    }

    // Zero initialization
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            res(i,j) = value_type{0};
        }
    }

    // Main computation with optimized blocking
    alignas(64) value_type lhs_cache[L1_BLOCK][L1_BLOCK];
    alignas(64) value_type rhs_cache[L1_BLOCK][MINI_BLOCK];

    for (int i0 = 0; i0 < M; i0 += L1_BLOCK) {
        const int imax = std::min(i0 + L1_BLOCK, M);
        
        for (int k0 = 0; k0 < K; k0 += L1_BLOCK) {
            const int kmax = std::min(k0 + L1_BLOCK, K);
            
            // Cache lhs block
            for (int i = i0; i < imax; ++i) {
                for (int k = k0; k < kmax; ++k) {
                    lhs_cache[i-i0][k-k0] = lhs(i,k);
                }
            }
            
            for (int j0 = 0; j0 < N; j0 += MINI_BLOCK) {
                const int jmax = std::min(j0 + MINI_BLOCK, N);
                
                // Process mini-blocks
                for (int i = i0; i < imax; i += MICRO_BLOCK) {
                    const int iimax = std::min(i + MICRO_BLOCK, imax);
                    
                    alignas(64) value_type local_acc[MICRO_BLOCK][MINI_BLOCK] = {{}};
                    
                    for (int k = k0; k < kmax; ++k) {
                        // Cache rhs values
                        for (int j = j0; j < jmax; ++j) {
                            rhs_cache[0][j-j0] = rhs(k,j);
                        }
                        
                        // Compute micro-block
                        for (int ii = i; ii < iimax; ++ii) {
                            const value_type lhs_val = lhs_cache[ii-i0][k-k0];
                            for (int j = j0; j < jmax; ++j) {
                                local_acc[ii-i][j-j0] += lhs_val * rhs_cache[0][j-j0];
                            }
                        }
                    }
                    
                    // Update result
                    for (int ii = i; ii < iimax; ++ii) {
                        for (int j = j0; j < jmax; ++j) {
                            res(ii,j) += local_acc[ii-i][j-j0];
                        }
                    }
                }
            }
        }
    }
    
    return res;
}
