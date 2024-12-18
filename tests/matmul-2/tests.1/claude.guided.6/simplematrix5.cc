
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int m = lhs.rows();
    const int n = rhs.columns();
    const int p = lhs.columns();  // Changed variable name from k to p to avoid confusion
    
    SimpleMatrix res{m, n};

    // Smaller block size to reduce register pressure
    constexpr int BLOCK_SIZE = 16;

    // Initialize result matrix
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            res(i,j) = SimpleMatrix::value_type{0};
        }
    }

    // Blocked matrix multiplication with reduced register pressure
    for (int ii = 0; ii < m; ii += BLOCK_SIZE) {
        const int i_end = std::min(ii + BLOCK_SIZE, m);
        
        for (int jj = 0; jj < n; jj += BLOCK_SIZE) {
            const int j_end = std::min(jj + BLOCK_SIZE, n);
            
            for (int kk = 0; kk < p; kk += BLOCK_SIZE) {
                const int k_end = std::min(kk + BLOCK_SIZE, p);
                
                for (int i = ii; i < i_end; ++i) {
                    for (int j = jj; j < j_end; ++j) {
                        SimpleMatrix::value_type temp = res(i,j);
                        
                        // Simple inner loop for better register allocation
                        for (int k = kk; k < k_end; ++k) {
                            temp += lhs(i,k) * rhs(k,j);
                        }
                        
                        res(i,j) = temp;
                    }
                }
            }
        }
    }
    
    return res;
}
