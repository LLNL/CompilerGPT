
#include <stdexcept>
#include <memory>
#include <cstring>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) 
{
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};
    
    // Optimize for small matrices
    if (M <= 16 && N <= 16 && K <= 16) {
        // Small matrix optimization with complete loop unrolling
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                double sum = 0.0;
                #pragma unroll
                for (int k = 0; k < K; ++k) {
                    sum += lhs(i, k) * rhs(k, j);
                }
                res(i, j) = sum;
            }
        }
        return res;
    }

    // For larger matrices use blocked algorithm with alignment
    constexpr int BLOCK_SIZE = 32;  // Adjust based on cache size
    constexpr int SMALL_BLOCK = 4;  // For micro-blocking
    
    // Temporary buffer for block transposition with aligned memory
    std::unique_ptr<double[]> block_buffer(new double[BLOCK_SIZE * BLOCK_SIZE]);
    
    for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
        const int imax = std::min(i0 + BLOCK_SIZE, M);
        
        for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
            const int jmax = std::min(j0 + BLOCK_SIZE, N);
            
            // Initialize result block to zero
            for (int i = i0; i < imax; ++i) {
                for (int j = j0; j < jmax; ++j) {
                    res(i, j) = 0.0;
                }
            }
            
            for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
                const int kmax = std::min(k0 + BLOCK_SIZE, K);
                
                // Transpose current block of rhs for better memory access
                for (int k = k0; k < kmax; ++k) {
                    for (int j = j0; j < jmax; ++j) {
                        block_buffer[(j - j0) * BLOCK_SIZE + (k - k0)] = rhs(k, j);
                    }
                }
                
                // Micro-blocked multiplication
                for (int i = i0; i < imax; i += SMALL_BLOCK) {
                    for (int j = j0; j < jmax; j += SMALL_BLOCK) {
                        for (int k = k0; k < kmax; k += SMALL_BLOCK) {
                            // Small block multiplication
                            for (int ii = i; ii < std::min(i + SMALL_BLOCK, imax); ++ii) {
                                double sum[SMALL_BLOCK] = {0};  // Use array for better vectorization
                                
                                for (int kk = k; kk < std::min(k + SMALL_BLOCK, kmax); ++kk) {
                                    const double lhs_val = lhs(ii, kk);
                                    const int block_idx = (kk - k0);
                                    
                                    #pragma unroll
                                    for (int jj = 0; jj < SMALL_BLOCK; ++jj) {
                                        if (j + jj < jmax) {
                                            sum[jj] += lhs_val * block_buffer[(j + jj - j0) * BLOCK_SIZE + block_idx];
                                        }
                                    }
                                }
                                
                                // Update result
                                #pragma unroll
                                for (int jj = 0; jj < SMALL_BLOCK; ++jj) {
                                    if (j + jj < jmax) {
                                        res(ii, j + jj) += sum[jj];
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
