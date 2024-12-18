
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

    // Cache line optimization - process in blocks
    constexpr int BLOCK_SIZE = 32;
    
    // Process matrix in blocks for better cache utilization
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            double temp = 0.0;
            // Split inner loop to help vectorization
            for (int k = 0; k < lhs.columns(); k += 4) {
                if (k + 3 < lhs.columns()) {
                    temp += lhs(i, k) * rhs(k, j) +
                           lhs(i, k+1) * rhs(k+1, j) +
                           lhs(i, k+2) * rhs(k+2, j) +
                           lhs(i, k+3) * rhs(k+3, j);
                } else {
                    // Handle remaining elements
                    for (; k < lhs.columns(); ++k) {
                        temp += lhs(i, k) * rhs(k, j);
                    }
                }
            }
            res(i,j) = temp;
        }
    }

    return res;
}
