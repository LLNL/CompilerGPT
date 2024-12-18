
#include <stdexcept>
#include <type_traits>
#include "simplematrix.h"

namespace {
    // Tune these based on your CPU's L1/L2 cache size
    constexpr int BLOCK_SIZE_M = 96;  // Increased for better cache utilization
    constexpr int BLOCK_SIZE_N = 96;
    constexpr int BLOCK_SIZE_K = 96;
    constexpr int MICRO_BLOCK_SIZE = 16;  // New micro-blocking size
    
    // Micro-kernel for multiplying small blocks that fit in L1 cache
    inline void micro_kernel_16x16(const SimpleMatrix& lhs, const SimpleMatrix& rhs,
                                 SimpleMatrix& res, int i, int j, int k) {
        // Load and cache 16 values from lhs
        const double lhs_0  = lhs(i+0, k);
        const double lhs_1  = lhs(i+1, k);
        const double lhs_2  = lhs(i+2, k);
        const double lhs_3  = lhs(i+3, k);
        const double lhs_4  = lhs(i+4, k);
        const double lhs_5  = lhs(i+5, k);
        const double lhs_6  = lhs(i+6, k);
        const double lhs_7  = lhs(i+7, k);
        const double lhs_8  = lhs(i+8, k);
        const double lhs_9  = lhs(i+9, k);
        const double lhs_10 = lhs(i+10, k);
        const double lhs_11 = lhs(i+11, k);
        const double lhs_12 = lhs(i+12, k);
        const double lhs_13 = lhs(i+13, k);
        const double lhs_14 = lhs(i+14, k);
        const double lhs_15 = lhs(i+15, k);

        // Cache rhs values to minimize memory access
        const double rhs_0  = rhs(k, j+0);
        const double rhs_1  = rhs(k, j+1);
        const double rhs_2  = rhs(k, j+2);
        const double rhs_3  = rhs(k, j+3);
        const double rhs_4  = rhs(k, j+4);
        const double rhs_5  = rhs(k, j+5);
        const double rhs_6  = rhs(k, j+6);
        const double rhs_7  = rhs(k, j+7);
        const double rhs_8  = rhs(k, j+8);
        const double rhs_9  = rhs(k, j+9);
        const double rhs_10 = rhs(k, j+10);
        const double rhs_11 = rhs(k, j+11);
        const double rhs_12 = rhs(k, j+12);
        const double rhs_13 = rhs(k, j+13);
        const double rhs_14 = rhs(k, j+14);
        const double rhs_15 = rhs(k, j+15);

        // Perform multiplication with maximum register usage
        #define UPDATE_ROW(i) \
            res(i+0,j+0)  += lhs_##i * rhs_0; \
            res(i+0,j+1)  += lhs_##i * rhs_1; \
            res(i+0,j+2)  += lhs_##i * rhs_2; \
            res(i+0,j+3)  += lhs_##i * rhs_3; \
            res(i+0,j+4)  += lhs_##i * rhs_4; \
            res(i+0,j+5)  += lhs_##i * rhs_5; \
            res(i+0,j+6)  += lhs_##i * rhs_6; \
            res(i+0,j+7)  += lhs_##i * rhs_7; \
            res(i+0,j+8)  += lhs_##i * rhs_8; \
            res(i+0,j+9)  += lhs_##i * rhs_9; \
            res(i+0,j+10) += lhs_##i * rhs_10; \
            res(i+0,j+11) += lhs_##i * rhs_11; \
            res(i+0,j+12) += lhs_##i * rhs_12; \
            res(i+0,j+13) += lhs_##i * rhs_13; \
            res(i+0,j+14) += lhs_##i * rhs_14; \
            res(i+0,j+15) += lhs_##i * rhs_15;

        UPDATE_ROW(0);  UPDATE_ROW(1);  UPDATE_ROW(2);  UPDATE_ROW(3);
        UPDATE_ROW(4);  UPDATE_ROW(5);  UPDATE_ROW(6);  UPDATE_ROW(7);
        UPDATE_ROW(8);  UPDATE_ROW(9);  UPDATE_ROW(10); UPDATE_ROW(11);
        UPDATE_ROW(12); UPDATE_ROW(13); UPDATE_ROW(14); UPDATE_ROW(15);

        #undef UPDATE_ROW
    }

    // Process a block with micro-kernels
    template<bool check_bounds = false>
    inline void process_block(const SimpleMatrix& lhs, const SimpleMatrix& rhs,
                            SimpleMatrix& res, int i0, int j0, int k0,
                            int imax, int jmax, int kmax) {
        for (int i = i0; i < (check_bounds ? imax : i0 + BLOCK_SIZE_M); i += MICRO_BLOCK_SIZE) {
            for (int k = k0; k < (check_bounds ? kmax : k0 + BLOCK_SIZE_K); ++k) {
                for (int j = j0; j < (check_bounds ? jmax : j0 + BLOCK_SIZE_N); j += MICRO_BLOCK_SIZE) {
                    if (!check_bounds || (i + MICRO_BLOCK_SIZE <= imax && j + MICRO_BLOCK_SIZE <= jmax)) {
                        micro_kernel_16x16(lhs, rhs, res, i, j, k);
                    } else {
                        // Handle edge cases
                        for (int ii = i; ii < std::min(i + MICRO_BLOCK_SIZE, imax); ++ii) {
                            const double lhs_val = lhs(ii, k);
                            for (int jj = j; jj < std::min(j + MICRO_BLOCK_SIZE, jmax); ++jj) {
                                res(ii, jj) += lhs_val * rhs(k, jj);
                            }
                        }
                    }
                }
            }
        }
    }
}

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) 
{
    const int m = lhs.rows();
    const int n = rhs.columns();
    const int p = lhs.columns();
    
    if (p != rhs.rows())
        throw std::runtime_error{"Matrix dimensions mismatch"};

    SimpleMatrix res{m, n};

    // Fast initialization using pointer arithmetic if available
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; j += 4) {
            if (j + 3 < n) {
                res(i,j) = res(i,j+1) = res(i,j+2) = res(i,j+3) = 0;
            } else {
                for (int jj = j; jj < n; ++jj) {
                    res(i,jj) = 0;
                }
            }
        }
    }

    // Process full blocks without bound checking
    for (int i0 = 0; i0 + BLOCK_SIZE_M <= m; i0 += BLOCK_SIZE_M) {
        for (int k0 = 0; k0 + BLOCK_SIZE_K <= p; k0 += BLOCK_SIZE_K) {
            for (int j0 = 0; j0 + BLOCK_SIZE_N <= n; j0 += BLOCK_SIZE_N) {
                process_block<false>(lhs, rhs, res, i0, j0, k0, m, n, p);
            }
        }
    }

    // Handle remaining blocks with bound checking
    if (m % BLOCK_SIZE_M || n % BLOCK_SIZE_N || p % BLOCK_SIZE_K) {
        for (int i0 = 0; i0 < m; i0 += BLOCK_SIZE_M) {
            for (int k0 = 0; k0 < p; k0 += BLOCK_SIZE_K) {
                for (int j0 = 0; j0 < n; j0 += BLOCK_SIZE_N) {
                    if (i0 + BLOCK_SIZE_M > m || k0 + BLOCK_SIZE_K > p || j0 + BLOCK_SIZE_N > n) {
                        process_block<true>(lhs, rhs, res, i0, j0, k0, m, n, p);
                    }
                }
            }
        }
    }

    return res;
}
