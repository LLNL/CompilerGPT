
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Even smaller block size to reduce register pressure
    constexpr int BLOCK_SIZE = 8;
    
    // Mini-block for inner loop unrolling
    constexpr int MINI_BLOCK = 4;

    for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
        const int imax = std::min(i0 + BLOCK_SIZE, M);
        
        for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
            const int jmax = std::min(j0 + BLOCK_SIZE, N);
            
            // Initialize current block
            for (int i = i0; i < imax; ++i) {
                for (int j = j0; j < jmax; ++j) {
                    res(i,j) = SimpleMatrix::value_type{0};
                }
            }

            // Process block
            for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
                const int kmax = std::min(k0 + BLOCK_SIZE, K);
                
                for (int i = i0; i < imax; ++i) {
                    for (int j = j0; j < jmax; ++j) {
                        SimpleMatrix::value_type sum = res(i,j);
                        
                        // Process elements in mini-blocks with manual unrolling
                        int k = k0;
                        for (; k + MINI_BLOCK <= kmax; k += MINI_BLOCK) {
                            // Load values once
                            const SimpleMatrix::value_type lhs0 = lhs(i,k);
                            const SimpleMatrix::value_type lhs1 = lhs(i,k+1);
                            const SimpleMatrix::value_type lhs2 = lhs(i,k+2);
                            const SimpleMatrix::value_type lhs3 = lhs(i,k+3);
                            
                            const SimpleMatrix::value_type rhs0 = rhs(k,j);
                            const SimpleMatrix::value_type rhs1 = rhs(k+1,j);
                            const SimpleMatrix::value_type rhs2 = rhs(k+2,j);
                            const SimpleMatrix::value_type rhs3 = rhs(k+3,j);
                            
                            // Accumulate products
                            sum += lhs0 * rhs0;
                            sum += lhs1 * rhs1;
                            sum += lhs2 * rhs2;
                            sum += lhs3 * rhs3;
                        }
                        
                        // Handle remaining elements
                        for (; k < kmax; ++k) {
                            sum += lhs(i,k) * rhs(k,j);
                        }
                        
                        res(i,j) = sum;
                    }
                }
            }
        }
    }
    return res;
}
