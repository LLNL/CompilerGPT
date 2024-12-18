
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int m = lhs.rows();
    const int n = rhs.columns();
    const int k = lhs.columns();
    
    SimpleMatrix res{m, n};

    // Block size chosen to fit L1 cache
    constexpr int BLOCK_I = 32;
    constexpr int BLOCK_J = 32;

    // Block-wise multiplication to reduce register pressure
    for (int i0 = 0; i0 < m; i0 += BLOCK_I) {
        const int imax = std::min(i0 + BLOCK_I, m);
        for (int j0 = 0; j0 < n; j0 += BLOCK_J) {
            const int jmax = std::min(j0 + BLOCK_J, n);
            
            // Process smaller blocks to reduce register pressure
            for (int i = i0; i < imax; ++i) {
                for (int j = j0; j < jmax; ++j) {
                    SimpleMatrix::value_type sum = SimpleMatrix::value_type{0};
                    
                    // Process elements in chunks of 8 to balance unrolling and register pressure
                    int k = 0;
                    for (; k + 7 < lhs.columns(); k += 8) {
                        sum += lhs(i, k) * rhs(k, j) +
                              lhs(i, k+1) * rhs(k+1, j) +
                              lhs(i, k+2) * rhs(k+2, j) +
                              lhs(i, k+3) * rhs(k+3, j);
                        sum += lhs(i, k+4) * rhs(k+4, j) +
                              lhs(i, k+5) * rhs(k+5, j) +
                              lhs(i, k+6) * rhs(k+6, j) +
                              lhs(i, k+7) * rhs(k+7, j);
                    }
                    
                    // Handle remaining elements
                    for (; k < lhs.columns(); ++k) {
                        sum += lhs(i, k) * rhs(k, j);
                    }
                    
                    res(i,j) = sum;
                }
            }
        }
    }

    return res;
}
