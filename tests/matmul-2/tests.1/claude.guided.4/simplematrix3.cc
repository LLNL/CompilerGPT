
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

    // Process matrix multiplication with explicit accumulator type
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            SimpleMatrix::value_type temp = SimpleMatrix::value_type{0};
            
            // Unroll the inner loop by factor of 4 for better pipelining
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
