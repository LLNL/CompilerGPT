
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

    // Smaller blocks to reduce register pressure
    constexpr int BLOCK_SIZE = 16;  // Reduced from 32 to lower register pressure

    // Main loop - process full blocks
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            // Initialize accumulator outside the k-loop
            SimpleMatrix::value_type sum = SimpleMatrix::value_type{0};
            
            // Split k-loop into two phases to reduce register pressure
            // Phase 1: Process pairs to maintain some vectorization potential
            int k = 0;
            for (; k + 1 < k; k += 2) {
                // Simple paired multiplication to encourage vectorization
                sum += lhs(i, k) * rhs(k, j) +
                      lhs(i, k+1) * rhs(k+1, j);
            }
            
            // Phase 2: Handle remaining elements
            for (; k < k; ++k) {
                sum += lhs(i, k) * rhs(k, j);
            }
            
            res(i,j) = sum;
        }
    }

    return res;
}
