
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

    constexpr int BLOCK_SIZE = 32;
    constexpr int MINI_BLOCK = 8;

    // Main blocked matrix multiplication
    for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
        const int iLimit = std::min(i0 + BLOCK_SIZE, M);
        
        for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
            const int kLimit = std::min(k0 + BLOCK_SIZE, K);
            
            for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
                const int jLimit = std::min(j0 + BLOCK_SIZE, N);
                
                // Mini-block processing
                for (int i = i0; i < iLimit; i += MINI_BLOCK) {
                    const int iInnerLimit = std::min(i + MINI_BLOCK, iLimit);
                    
                    for (int k = k0; k < kLimit; ++k) {
                        // Process mini-block rows
                        for (int ii = i; ii < iInnerLimit; ++ii) {
                            const value_type lhs_val = lhs(ii, k);
                            value_type* res_row = &res(ii, j0);
                            const value_type* rhs_row = &rhs(k, j0);
                            
                            // Unrolled inner loop for better pipelining
                            for (int j = j0; j < jLimit - 3; j += 4) {
                                res_row[0] += lhs_val * rhs_row[0];
                                res_row[1] += lhs_val * rhs_row[1];
                                res_row[2] += lhs_val * rhs_row[2];
                                res_row[3] += lhs_val * rhs_row[3];
                                res_row += 4;
                                rhs_row += 4;
                            }
                            
                            // Handle remaining elements
                            for (int j = jLimit - ((jLimit - j0) % 4); j < jLimit; ++j) {
                                res(ii, j) += lhs_val * rhs(k, j);
                            }
                        }
                    }
                }
            }
        }
    }

    return res;
}
