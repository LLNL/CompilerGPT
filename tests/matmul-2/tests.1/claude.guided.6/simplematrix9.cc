
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

    // Simple initialization
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            res(i,j) = SimpleMatrix::value_type{0};
        }
    }

    // Main computation with focus on vectorization
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            SimpleMatrix::value_type sum = SimpleMatrix::value_type{0};
            
            // Single loop for better vectorization
            #pragma clang loop vectorize(assume_safety)
            for (int k = 0; k < p; ++k) {
                sum += lhs(i,k) * rhs(k,j);
            }
            
            res(i,j) = sum;
        }
    }
    
    return res;
}
