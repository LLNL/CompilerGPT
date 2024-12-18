
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int m = lhs.rows();
    const int n = rhs.columns();
    const int k = lhs.columns();
    
    SimpleMatrix res{m, n};

    // Cache blocking size
    constexpr int BLOCK_SIZE = 32;

    // Initialize to zero first
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            res(i,j) = SimpleMatrix::value_type{0};
        }
    }

    // Blocked matrix multiplication with accumulator
    for (int ii = 0; ii < m; ii += BLOCK_SIZE) {
        for (int jj = 0; jj < n; jj += BLOCK_SIZE) {
            for (int kk = 0; kk < k; kk += BLOCK_SIZE) {
                const int i_end = std::min(ii + BLOCK_SIZE, m);
                const int j_end = std::min(jj + BLOCK_SIZE, n);
                const int k_end = std::min(kk + BLOCK_SIZE, k);

                for (int i = ii; i < i_end; ++i) {
                    for (int j = jj; j < j_end; ++j) {
                        SimpleMatrix::value_type temp = res(i,j);
                        // Inner loop with manual unrolling
                        for (int k = kk; k < k_end - 3; k += 4) {
                            temp += lhs(i,k) * rhs(k,j) +
                                   lhs(i,k+1) * rhs(k+1,j) +
                                   lhs(i,k+2) * rhs(k+2,j) +
                                   lhs(i,k+3) * rhs(k+3,j);
                        }
                        // Handle remaining elements
                        for (int k = k_end - (k_end-kk)%4; k < k_end; ++k) {
                            temp += lhs(i,k) * rhs(k,j);
                        }
                        res(i,j) = temp;
                    }
                }
            }
        }
    }
    return res;
}
