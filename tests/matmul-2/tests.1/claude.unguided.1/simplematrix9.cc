
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

    // Process matrix in 4x4 blocks for better cache utilization
    const int Mlim = M & ~3;  // Round down to multiple of 4
    const int Nlim = N & ~3;
    const int Klim = K & ~3;  // Round down to multiple of 4

    // Main computation loop with 4x4 blocks
    for (int i = 0; i < Mlim; i += 4) {
        for (int j = 0; j < Nlim; j += 4) {
            // Initialize accumulators for 4x4 block
            SimpleMatrix::value_type sum[4][4] = {{0}};
            SimpleMatrix::value_type temp[4][4] = {{0}};

            // Process K dimension
            for (int k = 0; k < Klim; k += 4) {
                // Cache lhs values for current 4x4 block
                SimpleMatrix::value_type lhs_cache[4][4];
                SimpleMatrix::value_type rhs_cache[4][4];

                // Load block into cache
                for (int ii = 0; ii < 4; ++ii) {
                    for (int kk = 0; kk < 4; ++kk) {
                        lhs_cache[ii][kk] = lhs(i + ii, k + kk);
                        rhs_cache[kk][ii] = rhs(k + kk, j + ii);
                    }
                }

                // Compute 4x4 block multiplication
                for (int ii = 0; ii < 4; ++ii) {
                    for (int jj = 0; jj < 4; ++jj) {
                        SimpleMatrix::value_type& s = sum[ii][jj];
                        SimpleMatrix::value_type& t = temp[ii][jj];
                        
                        // Split computation to break dependencies
                        const SimpleMatrix::value_type a0 = lhs_cache[ii][0];
                        const SimpleMatrix::value_type a1 = lhs_cache[ii][1];
                        const SimpleMatrix::value_type a2 = lhs_cache[ii][2];
                        const SimpleMatrix::value_type a3 = lhs_cache[ii][3];
                        
                        const SimpleMatrix::value_type b0 = rhs_cache[0][jj];
                        const SimpleMatrix::value_type b1 = rhs_cache[1][jj];
                        const SimpleMatrix::value_type b2 = rhs_cache[2][jj];
                        const SimpleMatrix::value_type b3 = rhs_cache[3][jj];

                        // Split computations to enable instruction-level parallelism
                        s += a0 * b0 + a1 * b1;
                        t += a2 * b2 + a3 * b3;
                    }
                }
            }

            // Handle remaining K elements
            for (int k = Klim; k < K; ++k) {
                for (int ii = 0; ii < 4; ++ii) {
                    const SimpleMatrix::value_type lhs_val = lhs(i + ii, k);
                    for (int jj = 0; jj < 4; ++jj) {
                        sum[ii][jj] += lhs_val * rhs(k, j + jj);
                    }
                }
            }

            // Store results combining both accumulators
            for (int ii = 0; ii < 4; ++ii) {
                for (int jj = 0; jj < 4; ++jj) {
                    res(i + ii, j + jj) = sum[ii][jj] + temp[ii][jj];
                }
            }
        }
    }

    // Handle remaining rows
    for (int i = Mlim; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            SimpleMatrix::value_type sum = 0;
            for (int k = 0; k < K; k += 4) {
                if (k + 3 < K) {
                    sum += lhs(i, k) * rhs(k, j) +
                          lhs(i, k+1) * rhs(k+1, j) +
                          lhs(i, k+2) * rhs(k+2, j) +
                          lhs(i, k+3) * rhs(k+3, j);
                } else {
                    for (int kk = k; kk < K; ++kk) {
                        sum += lhs(i, kk) * rhs(kk, j);
                    }
                }
            }
            res(i, j) = sum;
        }
    }

    // Handle remaining columns
    for (int i = 0; i < Mlim; ++i) {
        for (int j = Nlim; j < N; ++j) {
            SimpleMatrix::value_type sum = 0;
            for (int k = 0; k < K; k += 4) {
                if (k + 3 < K) {
                    sum += lhs(i, k) * rhs(k, j) +
                          lhs(i, k+1) * rhs(k+1, j) +
                          lhs(i, k+2) * rhs(k+2, j) +
                          lhs(i, k+3) * rhs(k+3, j);
                } else {
                    for (int kk = k; kk < K; ++kk) {
                        sum += lhs(i, kk) * rhs(kk, j);
                    }
                }
            }
            res(i, j) = sum;
        }
    }

    return res;
}
