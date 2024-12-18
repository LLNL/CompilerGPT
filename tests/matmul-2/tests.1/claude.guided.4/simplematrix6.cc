
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int m = lhs.rows();
    const int n = rhs.columns();
    const int cols = lhs.columns();  // Renamed to avoid confusion with loop variable k
    
    SimpleMatrix res{m, n};

    // Main loop - process each element
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            // Initialize accumulator with correct type
            SimpleMatrix::value_type sum = SimpleMatrix::value_type{0};
            
            // Split k-loop into two phases to reduce register pressure
            // Phase 1: Process pairs to maintain some vectorization potential
            int k = 0;
            for (; k + 1 < cols; k += 2) {
                // Simple paired multiplication to encourage vectorization
                SimpleMatrix::value_type term1 = lhs(i, k) * rhs(k, j);
                SimpleMatrix::value_type term2 = lhs(i, k+1) * rhs(k+1, j);
                sum += term1 + term2;
            }
            
            // Phase 2: Handle remaining elements
            for (; k < cols; ++k) {
                sum += lhs(i, k) * rhs(k, j);
            }
            
            res(i,j) = sum;
        }
    }

    return res;
}
