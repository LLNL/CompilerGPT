
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) 
{
    // Cache dimensions to avoid repeated function calls
    const int m = lhs.rows();
    const int n = lhs.columns();
    const int p = rhs.columns();

    if (n != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    SimpleMatrix res{m, p};

    // Loop ordering optimization (i,k,j) for better cache usage
    for (int i = 0; i < m; ++i) {
        for (int k = 0; k < n; ++k) {
            const double lhs_ik = lhs(i, k);  // Cache this value
            
            // Inner loop now performs scalar multiplication and addition
            // Better cache locality for accessing rhs matrix
            for (int j = 0; j < p; ++j) {
                res(i,j) += lhs_ik * rhs(k, j);
            }
        }
    }

    return res;
}
