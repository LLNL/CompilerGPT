
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int m = lhs.rows();
    const int n = rhs.columns();
    const int p = lhs.columns();
    
    SimpleMatrix res{m, n};

    // Simple initialization - compiler can better optimize this
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            res(i,j) = SimpleMatrix::value_type{0};
        }
    }

    // Simple implementation focusing on minimal register pressure
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            SimpleMatrix::value_type sum = SimpleMatrix::value_type{0};
            
            // Keep accumulator in register, minimize memory access
            const int p_minus_3 = p - 3;
            int k = 0;
            
            // Main loop with manual unrolling by 4
            for (; k < p_minus_3; k += 4) {
                sum += lhs(i,k) * rhs(k,j) +
                      lhs(i,k+1) * rhs(k+1,j) +
                      lhs(i,k+2) * rhs(k+2,j) +
                      lhs(i,k+3) * rhs(k+3,j);
            }
            
            // Handle remaining elements
            for (; k < p; ++k) {
                sum += lhs(i,k) * rhs(k,j);
            }
            
            res(i,j) = sum;
        }
    }
    
    return res;
}
