
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

    // Blocking parameters tuned for common L1 cache sizes
    constexpr int BLOCK_I = 8;  // Increased block size
    constexpr int BLOCK_J = 8;  // Increased block size
    constexpr int BLOCK_K = 8;  // Added K dimension blocking
    
    // Pre-initialize result matrix to zero
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            res(i, j) = 0;
        }
    }

    // Triple blocking for better cache utilization
    for (int i0 = 0; i0 < M; i0 += BLOCK_I) {
        const int imax = (i0 + BLOCK_I < M) ? i0 + BLOCK_I : M;
        
        for (int j0 = 0; j0 < N; j0 += BLOCK_J) {
            const int jmax = (j0 + BLOCK_J < N) ? j0 + BLOCK_J : N;
            
            for (int k0 = 0; k0 < K; k0 += BLOCK_K) {
                const int kmax = (k0 + BLOCK_K < K) ? k0 + BLOCK_K : K;

                // Cache block values from rhs
                SimpleMatrix::value_type rhs_cache[BLOCK_K][BLOCK_J];
                for (int k = k0; k < kmax; ++k) {
                    for (int j = j0; j < jmax; ++j) {
                        rhs_cache[k - k0][j - j0] = rhs(k, j);
                    }
                }

                // Cache block values from lhs
                SimpleMatrix::value_type lhs_cache[BLOCK_I][BLOCK_K];
                for (int i = i0; i < imax; ++i) {
                    for (int k = k0; k < kmax; ++k) {
                        lhs_cache[i - i0][k - k0] = lhs(i, k);
                    }
                }

                // Process block with super-aggressive unrolling
                for (int i = i0; i < imax; ++i) {
                    const int i_idx = i - i0;
                    
                    for (int j = j0; j < jmax; j += 4) {
                        const int j_idx = j - j0;
                        
                        // Early exit check for remaining columns
                        if (j + 3 >= jmax) break;

                        // Accumulate in local variables for better register usage
                        SimpleMatrix::value_type sum0 = res(i, j);
                        SimpleMatrix::value_type sum1 = res(i, j+1);
                        SimpleMatrix::value_type sum2 = res(i, j+2);
                        SimpleMatrix::value_type sum3 = res(i, j+3);

                        // Unrolled k-loop
                        for (int k = k0; k < kmax; k += 2) {
                            const int k_idx = k - k0;
                            
                            if (k + 1 >= kmax) {
                                // Handle single k iteration
                                const SimpleMatrix::value_type lhs_val = lhs_cache[i_idx][k_idx];
                                sum0 += lhs_val * rhs_cache[k_idx][j_idx];
                                sum1 += lhs_val * rhs_cache[k_idx][j_idx+1];
                                sum2 += lhs_val * rhs_cache[k_idx][j_idx+2];
                                sum3 += lhs_val * rhs_cache[k_idx][j_idx+3];
                            } else {
                                // Handle paired k iterations
                                const SimpleMatrix::value_type lhs_val0 = lhs_cache[i_idx][k_idx];
                                const SimpleMatrix::value_type lhs_val1 = lhs_cache[i_idx][k_idx+1];
                                
                                sum0 += lhs_val0 * rhs_cache[k_idx][j_idx] +
                                       lhs_val1 * rhs_cache[k_idx+1][j_idx];
                                sum1 += lhs_val0 * rhs_cache[k_idx][j_idx+1] +
                                       lhs_val1 * rhs_cache[k_idx+1][j_idx+1];
                                sum2 += lhs_val0 * rhs_cache[k_idx][j_idx+2] +
                                       lhs_val1 * rhs_cache[k_idx+1][j_idx+2];
                                sum3 += lhs_val0 * rhs_cache[k_idx][j_idx+3] +
                                       lhs_val1 * rhs_cache[k_idx+1][j_idx+3];
                            }
                        }

                        // Store results
                        res(i, j) = sum0;
                        res(i, j+1) = sum1;
                        res(i, j+2) = sum2;
                        res(i, j+3) = sum3;
                    }

                    // Handle remaining columns
                    for (int j = jmax - (jmax - j0) % 4; j < jmax; ++j) {
                        SimpleMatrix::value_type sum = res(i, j);
                        for (int k = k0; k < kmax; ++k) {
                            sum += lhs_cache[i - i0][k - k0] * rhs_cache[k - k0][j - j0];
                        }
                        res(i, j) = sum;
                    }
                }
            }
        }
    }

    return res;
}
