
#include <stdexcept>
#include <type_traits>
#include <memory>
#include "simplematrix.h"

namespace {
    // Compile-time checks and constants
    constexpr int BLOCK_SIZE_M = 64;  // Adjusted for modern CPU cache sizes
    constexpr int BLOCK_SIZE_N = 64;
    constexpr int BLOCK_SIZE_K = 64;
    constexpr int UNROLL_FACTOR = 8;  // Increased unroll factor
    constexpr size_t L1_CACHE_LINE = 64; // Typical cache line size

    // Alignment helper
    template<typename T>
    inline T* align_ptr(T* ptr, size_t alignment) {
        return reinterpret_cast<T*>((reinterpret_cast<uintptr_t>(ptr) + alignment - 1) & ~(alignment - 1));
    }

    // Helper for small matrix multiplication (no blocking)
    template<int M, int N, int K>
    inline void multiply_small_block(const SimpleMatrix& lhs, const SimpleMatrix& rhs, 
                                   SimpleMatrix& res, int i0, int j0, int k0) {
        alignas(L1_CACHE_LINE) double lhs_cache[M][K];
        alignas(L1_CACHE_LINE) double rhs_cache[K][N];
        alignas(L1_CACHE_LINE) double res_cache[M][N] = {};

        // Prefetch data into aligned cache
        for (int i = 0; i < M; ++i)
            for (int k = 0; k < K; ++k)
                lhs_cache[i][k] = lhs(i0 + i, k0 + k);

        for (int k = 0; k < K; ++k)
            for (int j = 0; j < N; ++j)
                rhs_cache[k][j] = rhs(k0 + k, j0 + j);

        // Main computation with aggressive unrolling
        #define UNROLL8(i) \
            res_cache[i][j] += lhs_cache[i][k] * rhs_cache[k][j]; \
            res_cache[i][j+1] += lhs_cache[i][k] * rhs_cache[k][j+1]; \
            res_cache[i][j+2] += lhs_cache[i][k] * rhs_cache[k][j+2]; \
            res_cache[i][j+3] += lhs_cache[i][k] * rhs_cache[k][j+3]; \
            res_cache[i][j+4] += lhs_cache[i][k] * rhs_cache[k][j+4]; \
            res_cache[i][j+5] += lhs_cache[i][k] * rhs_cache[k][j+5]; \
            res_cache[i][j+6] += lhs_cache[i][k] * rhs_cache[k][j+6]; \
            res_cache[i][j+7] += lhs_cache[i][k] * rhs_cache[k][j+7];

        for (int k = 0; k < K; ++k) {
            for (int i = 0; i < M; ++i) {
                for (int j = 0; j < N; j += UNROLL_FACTOR) {
                    UNROLL8(i)
                }
            }
        }

        // Write back results
        for (int i = 0; i < M; ++i)
            for (int j = 0; j < N; ++j)
                res(i0 + i, j0 + j) += res_cache[i][j];

        #undef UNROLL8
    }
}

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) 
{
    const int m = lhs.rows();
    const int n = rhs.columns();
    const int p = lhs.columns();
    
    if (p != rhs.rows())
        throw std::runtime_error{"Matrix dimensions mismatch"};

    // Fast path for small matrices
    if (m <= 32 && n <= 32 && p <= 32) {
        SimpleMatrix res{m, n};
        multiply_small_block<32,32,32>(lhs, rhs, res, 0, 0, 0);
        return res;
    }

    SimpleMatrix res{m, n};

    // Aligned buffer for temporary computations
    const size_t temp_size = BLOCK_SIZE_M * BLOCK_SIZE_N * sizeof(double);
    std::unique_ptr<char[]> temp_buffer(new char[temp_size + L1_CACHE_LINE]);
    double* aligned_temp = reinterpret_cast<double*>(
        align_ptr(temp_buffer.get(), L1_CACHE_LINE));

    // Initialize result matrix using vectorizable pattern
    for (int i = 0; i < m; i++) {
        double* row = aligned_temp;
        for (int j = 0; j < n; ++j) {
            row[j] = 0.0;
        }
        for (int j = 0; j < n; ++j) {
            res(i,j) = row[j];
        }
    }

    // Block multiplication with prefetching
    #pragma GCC ivdep
    for (int i0 = 0; i0 < m; i0 += BLOCK_SIZE_M) {
        const int imax = std::min(i0 + BLOCK_SIZE_M, m);
        for (int k0 = 0; k0 < p; k0 += BLOCK_SIZE_K) {
            const int kmax = std::min(k0 + BLOCK_SIZE_K, p);
            
            // Prefetch next block
            if (k0 + BLOCK_SIZE_K < p) {
                for (int i = i0; i < imax; ++i) {
                    __builtin_prefetch(&lhs(i, k0 + BLOCK_SIZE_K), 0, 3);
                }
            }

            for (int j0 = 0; j0 < n; j0 += BLOCK_SIZE_N) {
                const int jmax = std::min(j0 + BLOCK_SIZE_N, n);
                
                // Process block
                multiply_small_block<BLOCK_SIZE_M, BLOCK_SIZE_N, BLOCK_SIZE_K>(
                    lhs, rhs, res, i0, j0, k0);
            }
        }
    }

    return res;
}
