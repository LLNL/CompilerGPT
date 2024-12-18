
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};
    using value_type = SimpleMatrix::value_type;

    // Fast initialization using single loop
    const int total_size = M * N;
    for (int idx = 0; idx < total_size; ++idx) {
        res(idx / N, idx % N) = 0;
    }

    // Cache-optimized block sizes
    constexpr int OUTER_BLOCK = 96;   // L2/L3 cache block
    constexpr int MIDDLE_BLOCK = 48;  // L1 cache block
    constexpr int INNER_BLOCK = 16;   // Register block
    constexpr int UNROLL = 4;         // Unrolling factor for innermost loop
    constexpr int ROW_PAIR = 2;       // Number of rows to process together

    // Outer blocking for L2/L3 cache
    for (int k0 = 0; k0 < K; k0 += OUTER_BLOCK) {
        const int kLimit = std::min(k0 + OUTER_BLOCK, K);
        
        for (int i0 = 0; i0 < M; i0 += OUTER_BLOCK) {
            const int iLimit = std::min(i0 + OUTER_BLOCK, M);
            
            // Middle blocking for L1 cache
            for (int k1 = k0; k1 < kLimit; k1 += MIDDLE_BLOCK) {
                const int kMidLimit = std::min(k1 + MIDDLE_BLOCK, kLimit);
                
                for (int i1 = i0; i1 < iLimit; i1 += MIDDLE_BLOCK) {
                    const int iMidLimit = std::min(i1 + MIDDLE_BLOCK, iLimit);

                    // Inner blocking for registers
                    for (int k = k1; k < kMidLimit; ++k) {
                        for (int i = i1; i < iMidLimit; i += ROW_PAIR) {
                            // Pre-fetch next iteration's data
                            const value_type lhs_ik = lhs(i, k);
                            const value_type lhs_i1k = (i + 1 < iMidLimit) ? lhs(i + 1, k) : 0;

                            // Process blocks of N dimension
                            for (int j = 0; j < N; j += INNER_BLOCK) {
                                const int jLimit = std::min(j + INNER_BLOCK, N);
                                
                                // Unrolled processing of elements
                                int j_block = j;
                                for (; j_block + UNROLL <= jLimit; j_block += UNROLL) {
                                    // Load rhs values
                                    const value_type rhs_k0 = rhs(k, j_block);
                                    const value_type rhs_k1 = rhs(k, j_block + 1);
                                    const value_type rhs_k2 = rhs(k, j_block + 2);
                                    const value_type rhs_k3 = rhs(k, j_block + 3);

                                    // First row updates
                                    res(i, j_block) += lhs_ik * rhs_k0;
                                    res(i, j_block + 1) += lhs_ik * rhs_k1;
                                    res(i, j_block + 2) += lhs_ik * rhs_k2;
                                    res(i, j_block + 3) += lhs_ik * rhs_k3;

                                    // Second row updates (if within bounds)
                                    if (i + 1 < iMidLimit) {
                                        res(i + 1, j_block) += lhs_i1k * rhs_k0;
                                        res(i + 1, j_block + 1) += lhs_i1k * rhs_k1;
                                        res(i + 1, j_block + 2) += lhs_i1k * rhs_k2;
                                        res(i + 1, j_block + 3) += lhs_i1k * rhs_k3;
                                    }
                                }

                                // Handle remaining elements
                                for (; j_block < jLimit; ++j_block) {
                                    const value_type rhs_val = rhs(k, j_block);
                                    res(i, j_block) += lhs_ik * rhs_val;
                                    if (i + 1 < iMidLimit) {
                                        res(i + 1, j_block) += lhs_i1k * rhs_val;
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
