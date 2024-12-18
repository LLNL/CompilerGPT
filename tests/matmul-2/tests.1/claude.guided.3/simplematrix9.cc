
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};
    using value_type = SimpleMatrix::value_type;

    // Process one row at a time to minimize register pressure
    for (int i = 0; i < M; ++i) {
        // Pre-fetch current row of lhs
        value_type row_i[2];  // Small fixed-size buffer
        
        // Process in pairs of columns when possible
        for (int j = 0; j < N-1; j += 2) {
            value_type sum0 = value_type{0};
            value_type sum1 = value_type{0};
            
            // Process in pairs of K elements
            int k = 0;
            for (; k < K-1; k += 2) {
                // Load pair of elements from lhs row
                row_i[0] = lhs(i,k);
                row_i[1] = lhs(i,k+1);
                
                // Compute contributions
                sum0 += row_i[0] * rhs(k,j) + row_i[1] * rhs(k+1,j);
                sum1 += row_i[0] * rhs(k,j+1) + row_i[1] * rhs(k+1,j+1);
            }
            
            // Handle remaining K element if any
            if (k < K) {
                const value_type last = lhs(i,k);
                sum0 += last * rhs(k,j);
                sum1 += last * rhs(k,j+1);
            }
            
            res(i,j) = sum0;
            res(i,j+1) = sum1;
        }
        
        // Handle last column if N is odd
        if (N % 2) {
            value_type sum = value_type{0};
            for (int k = 0; k < K; ++k) {
                sum += lhs(i,k) * rhs(k,N-1);
            }
            res(i,N-1) = sum;
        }
    }

    return res;
}
