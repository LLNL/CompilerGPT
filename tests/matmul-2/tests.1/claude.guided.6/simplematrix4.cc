
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int m = lhs.rows();
    const int n = rhs.columns();
    const int k = lhs.columns();
    
    SimpleMatrix res{m, n};

    // Smaller block size to reduce register pressure
    constexpr int BLOCK_SIZE = 16;

    // Single loop initialization
    const int total_size = m * n;
    for (int idx = 0; idx < total_size; ++idx) {
        res(idx / n, idx % n) = SimpleMatrix::value_type{0};
    }

    // Blocked matrix multiplication with reduced register pressure
    for (int ii = 0; ii < m; ii += BLOCK_SIZE) {
        const int i_end = std::min(ii + BLOCK_SIZE, m);
        
        for (int jj = 0; jj < n; jj += BLOCK_SIZE) {
            const int j_end = std::min(jj + BLOCK_SIZE, n);
            
            for (int i = ii; i < i_end; ++i) {
                for (int j = jj; j < j_end; ++j) {
                    // Move accumulator initialization here
                    SimpleMatrix::value_type temp = SimpleMatrix::value_type{0};
                    
                    // Single k-loop without unrolling to reduce register pressure
                    for (int k = 0; k < k; ++k) {
                        temp += lhs(i,k) * rhs(k,j);
                    }
                    
                    res(i,j) = temp;
                }
            }
        }
    }
    
    return res;
}
