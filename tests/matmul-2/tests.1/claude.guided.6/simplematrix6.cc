
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

    // Even smaller block size to reduce register pressure
    constexpr int BLOCK_SIZE = 8;

    // Initialize result matrix - single loop for better vectorization
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            res(i,j) = SimpleMatrix::value_type{0};
        }
    }

    // Process 2x2 blocks at a time to reduce register pressure
    for (int ii = 0; ii < m; ii += BLOCK_SIZE) {
        const int i_end = std::min(ii + BLOCK_SIZE, m);
        
        for (int jj = 0; jj < n; jj += BLOCK_SIZE) {
            const int j_end = std::min(jj + BLOCK_SIZE, n);

            // Preload block bounds
            const int i_size = i_end - ii;
            const int j_size = j_end - jj;
            
            // Process smaller chunks for better register allocation
            for (int k = 0; k < p; ++k) {
                for (int i = ii; i < i_end; ++i) {
                    // Cache lhs value for inner loop
                    const auto lhs_ik = lhs(i,k);
                    
                    for (int j = jj; j < j_end; ++j) {
                        res(i,j) += lhs_ik * rhs(k,j);
                    }
                }
            }
        }
    }
    
    return res;
}
