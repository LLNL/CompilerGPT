
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Smaller computation kernels
    constexpr int BLOCK_SIZE = 4;
    
    // Zero initialization
    double* const res_data = &res(0,0);
    for (int i = 0; i < M * N; ++i) {
        res_data[i] = 0.0;
    }

    // Blocked matrix multiplication with accumulator arrays
    for (int ii = 0; ii < M; ii += BLOCK_SIZE) {
        const int iEnd = std::min(ii + BLOCK_SIZE, M);
        
        for (int jj = 0; jj < N; jj += BLOCK_SIZE) {
            const int jEnd = std::min(jj + BLOCK_SIZE, N);
            
            // Local accumulators to reduce memory traffic
            double local_sums[BLOCK_SIZE][BLOCK_SIZE] = {};
            
            // Accumulate into local array
            for (int k = 0; k < K; ++k) {
                for (int i = 0; i < iEnd - ii; ++i) {
                    const double lhs_val = lhs(ii + i, k);
                    
                    for (int j = 0; j < jEnd - jj; ++j) {
                        local_sums[i][j] += lhs_val * rhs(k, jj + j);
                    }
                }
            }
            
            // Write back accumulated results
            for (int i = 0; i < iEnd - ii; ++i) {
                for (int j = 0; j < jEnd - jj; ++j) {
                    res(ii + i, jj + j) = local_sums[i][j];
                }
            }
        }
    }
    
    return res;
}
