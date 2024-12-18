
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) 
{
    const int m = lhs.rows();    // Cache matrix dimensions
    const int n = rhs.columns();
    const int p = lhs.columns();
    
    if (p != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    SimpleMatrix res{m, n};
    
    // Pre-initialize result matrix to zero
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            res(i,j) = 0;
        }
    }

    // Main multiplication loop with loop interchange for better cache locality
    for (int i = 0; i < m; ++i) {
        for (int k = 0; k < p; ++k) {
            const double lhs_ik = lhs(i,k);  // Cache the value
            for (int j = 0; j < n; ++j) {
                res(i,j) += lhs_ik * rhs(k,j);
            }
        }
    }

    return res;
}
