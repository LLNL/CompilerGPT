
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int m = lhs.rows();
    const int n = rhs.columns();
    const int cols = lhs.columns();
    
    SimpleMatrix res{m, n};

    #pragma clang loop vectorize(enable) interleave(enable)
    for (int i = 0; i < m; i++) {
        #pragma clang loop vectorize(enable) interleave(enable)
        for (int j = 0; j < n; j++) {
            SimpleMatrix::value_type sum = SimpleMatrix::value_type{0};
            
            // Main loop - handling 4 elements at once
            int k = 0;
            
            // Ensure proper alignment and type handling
            #pragma clang loop vectorize(enable) interleave(enable)
            for (; k + 3 < cols; k += 4) {
                // Explicit load to help compiler recognize pattern
                SimpleMatrix::value_type v1 = lhs(i, k);
                SimpleMatrix::value_type v2 = lhs(i, k+1);
                SimpleMatrix::value_type v3 = lhs(i, k+2);
                SimpleMatrix::value_type v4 = lhs(i, k+3);
                
                SimpleMatrix::value_type r1 = rhs(k, j);
                SimpleMatrix::value_type r2 = rhs(k+1, j);
                SimpleMatrix::value_type r3 = rhs(k+2, j);
                SimpleMatrix::value_type r4 = rhs(k+3, j);

                // Separate multiplications and additions to help register allocation
                sum += v1 * r1;
                sum += v2 * r2;
                sum += v3 * r3;
                sum += v4 * r4;
            }
            
            // Handle remaining elements
            for (; k < cols; ++k) {
                sum += lhs(i, k) * rhs(k, j);
            }
            
            res(i,j) = sum;
        }
    }

    return res;
}
