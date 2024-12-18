
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Using SimpleMatrix::value_type for proper data type
    using value_type = SimpleMatrix::value_type;
    
    // Zero initialization outside loops
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            res(i,j) = value_type{0};
        }
    }

    // Cache blocking with smaller blocks
    constexpr int BLOCK_SIZE = 16;

    for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
        const int iLimit = std::min(i0 + BLOCK_SIZE, M);
        
        for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
            const int jLimit = std::min(j0 + BLOCK_SIZE, N);
            
            for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
                const int kLimit = std::min(k0 + BLOCK_SIZE, K);

                for (int i = i0; i < iLimit; ++i) {
                    for (int j = j0; j < jLimit; ++j) {
                        value_type sum = res(i,j);
                        
                        // Inner product computation
                        for (int k = k0; k < kLimit; ++k) {
                            sum += lhs(i,k) * rhs(k,j);
                        }
                        
                        res(i,j) = sum;
                    }
                }
            }
        }
    }

    return res;
}
