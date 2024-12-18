
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

    // Initialize result matrix
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            res(i,j) = 0;
        }
    }

    // Tuned block sizes for better cache utilization
    constexpr int OUTER_BLOCK = 64;   // L2 cache block
    constexpr int MIDDLE_BLOCK = 32;  // L1 cache block
    constexpr int INNER_BLOCK = 8;    // Register block
    constexpr int UNROLL = 8;         // Unrolling factor

    // Outer blocking for L2 cache
    for (int i0 = 0; i0 < M; i0 += OUTER_BLOCK) {
        const int iLimit = std::min(i0 + OUTER_BLOCK, M);
        
        for (int k0 = 0; k0 < K; k0 += OUTER_BLOCK) {
            const int kLimit = std::min(k0 + OUTER_BLOCK, K);
            
            // Middle blocking for L1 cache
            for (int i1 = i0; i1 < iLimit; i1 += MIDDLE_BLOCK) {
                const int iMidLimit = std::min(i1 + MIDDLE_BLOCK, iLimit);
                
                for (int k1 = k0; k1 < kLimit; k1 += MIDDLE_BLOCK) {
                    const int kMidLimit = std::min(k1 + MIDDLE_BLOCK, kLimit);

                    // Process blocks for N dimension
                    for (int j = 0; j < N; j += INNER_BLOCK) {
                        const int jLimit = std::min(j + INNER_BLOCK, N);

                        // Inner blocking for registers
                        for (int i = i1; i < iMidLimit; i += 2) {
                            for (int k = k1; k < kMidLimit; ++k) {
                                const value_type lhs_i0k = lhs(i, k);
                                const value_type lhs_i1k = (i + 1 < iMidLimit) ? lhs(i + 1, k) : 0;

                                // Unrolled inner loop
                                int j_block = j;
                                for (; j_block + UNROLL <= jLimit; j_block += UNROLL) {
                                    // First row
                                    res(i,j_block) += lhs_i0k * rhs(k,j_block);
                                    res(i,j_block+1) += lhs_i0k * rhs(k,j_block+1);
                                    res(i,j_block+2) += lhs_i0k * rhs(k,j_block+2);
                                    res(i,j_block+3) += lhs_i0k * rhs(k,j_block+3);
                                    res(i,j_block+4) += lhs_i0k * rhs(k,j_block+4);
                                    res(i,j_block+5) += lhs_i0k * rhs(k,j_block+5);
                                    res(i,j_block+6) += lhs_i0k * rhs(k,j_block+6);
                                    res(i,j_block+7) += lhs_i0k * rhs(k,j_block+7);

                                    // Second row
                                    if (i + 1 < iMidLimit) {
                                        res(i+1,j_block) += lhs_i1k * rhs(k,j_block);
                                        res(i+1,j_block+1) += lhs_i1k * rhs(k,j_block+1);
                                        res(i+1,j_block+2) += lhs_i1k * rhs(k,j_block+2);
                                        res(i+1,j_block+3) += lhs_i1k * rhs(k,j_block+3);
                                        res(i+1,j_block+4) += lhs_i1k * rhs(k,j_block+4);
                                        res(i+1,j_block+5) += lhs_i1k * rhs(k,j_block+5);
                                        res(i+1,j_block+6) += lhs_i1k * rhs(k,j_block+6);
                                        res(i+1,j_block+7) += lhs_i1k * rhs(k,j_block+7);
                                    }
                                }

                                // Handle remaining elements
                                for (; j_block < jLimit; ++j_block) {
                                    res(i,j_block) += lhs_i0k * rhs(k,j_block);
                                    if (i + 1 < iMidLimit) {
                                        res(i+1,j_block) += lhs_i1k * rhs(k,j_block);
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
