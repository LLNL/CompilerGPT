
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

    // Smaller block size to reduce register pressure
    constexpr int BLOCK_SIZE = 16;
    // Mini-block for register blocking
    constexpr int REGISTER_BLOCK = 4;

    // Initialize with vector-friendly pattern
    for (int i = 0; i < M; ++i) {
        int j = 0;
        // Vector-friendly initialization
        for (; j + REGISTER_BLOCK <= N; j += REGISTER_BLOCK) {
            res(i,j) = 0.0;
            res(i,j+1) = 0.0;
            res(i,j+2) = 0.0;
            res(i,j+3) = 0.0;
        }
        // Cleanup
        for (; j < N; ++j) {
            res(i,j) = 0.0;
        }
    }

    // Blocked matrix multiplication with register blocking
    for (int ii = 0; ii < M; ii += BLOCK_SIZE) {
        const int iEnd = std::min(ii + BLOCK_SIZE, M);
        
        for (int kk = 0; kk < K; kk += BLOCK_SIZE) {
            const int kEnd = std::min(kk + BLOCK_SIZE, K);
            
            for (int jj = 0; jj < N; jj += BLOCK_SIZE) {
                const int jEnd = std::min(jj + BLOCK_SIZE, N);

                for (int i = ii; i < iEnd; ++i) {
                    for (int k = kk; k < kEnd; ++k) {
                        const double lhs_ik = lhs(i,k);
                        
                        int j = jj;
                        // Vector-friendly inner loop with register blocking
                        for (; j + REGISTER_BLOCK <= jEnd; j += REGISTER_BLOCK) {
                            double sum0 = res(i,j);
                            double sum1 = res(i,j+1);
                            double sum2 = res(i,j+2);
                            double sum3 = res(i,j+3);

                            sum0 += lhs_ik * rhs(k,j);
                            sum1 += lhs_ik * rhs(k,j+1);
                            sum2 += lhs_ik * rhs(k,j+2);
                            sum3 += lhs_ik * rhs(k,j+3);

                            res(i,j) = sum0;
                            res(i,j+1) = sum1;
                            res(i,j+2) = sum2;
                            res(i,j+3) = sum3;
                        }
                        // Cleanup remaining elements
                        for (; j < jEnd; ++j) {
                            res(i,j) += lhs_ik * rhs(k,j);
                        }
                    }
                }
            }
        }
    }
    return res;
}
