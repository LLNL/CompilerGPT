
#include <stdexcept>
#include <type_traits>
#include "simplematrix.h"

namespace {
    // Compile-time constants
    constexpr int BLOCK_SIZE_M = 64;
    constexpr int BLOCK_SIZE_N = 64;
    constexpr int BLOCK_SIZE_K = 64;
    constexpr int UNROLL_FACTOR = 8;

    // Helper for block multiplication
    template<int M, int N, int K>
    inline void multiply_block(const SimpleMatrix& lhs, const SimpleMatrix& rhs, 
                             SimpleMatrix& res, int i0, int j0, int k0) {
        const int imax = std::min(i0 + M, lhs.rows());
        const int jmax = std::min(j0 + N, rhs.columns());
        const int kmax = std::min(k0 + K, lhs.columns());

        // Process 8x8 blocks when possible
        for (int i = i0; i < imax; i += 8) {
            if (i + 7 < imax) {
                for (int k = k0; k < kmax; ++k) {
                    const double lhs_ik0 = lhs(i, k);
                    const double lhs_ik1 = lhs(i+1, k);
                    const double lhs_ik2 = lhs(i+2, k);
                    const double lhs_ik3 = lhs(i+3, k);
                    const double lhs_ik4 = lhs(i+4, k);
                    const double lhs_ik5 = lhs(i+5, k);
                    const double lhs_ik6 = lhs(i+6, k);
                    const double lhs_ik7 = lhs(i+7, k);

                    for (int j = j0; j < jmax; j += 8) {
                        if (j + 7 < jmax) {
                            // Load rhs values
                            const double rhs_kj0 = rhs(k, j);
                            const double rhs_kj1 = rhs(k, j+1);
                            const double rhs_kj2 = rhs(k, j+2);
                            const double rhs_kj3 = rhs(k, j+3);
                            const double rhs_kj4 = rhs(k, j+4);
                            const double rhs_kj5 = rhs(k, j+5);
                            const double rhs_kj6 = rhs(k, j+6);
                            const double rhs_kj7 = rhs(k, j+7);

                            // Compute 8x8 block
                            res(i,j)     += lhs_ik0 * rhs_kj0;
                            res(i,j+1)   += lhs_ik0 * rhs_kj1;
                            res(i,j+2)   += lhs_ik0 * rhs_kj2;
                            res(i,j+3)   += lhs_ik0 * rhs_kj3;
                            res(i,j+4)   += lhs_ik0 * rhs_kj4;
                            res(i,j+5)   += lhs_ik0 * rhs_kj5;
                            res(i,j+6)   += lhs_ik0 * rhs_kj6;
                            res(i,j+7)   += lhs_ik0 * rhs_kj7;

                            res(i+1,j)   += lhs_ik1 * rhs_kj0;
                            res(i+1,j+1) += lhs_ik1 * rhs_kj1;
                            res(i+1,j+2) += lhs_ik1 * rhs_kj2;
                            res(i+1,j+3) += lhs_ik1 * rhs_kj3;
                            res(i+1,j+4) += lhs_ik1 * rhs_kj4;
                            res(i+1,j+5) += lhs_ik1 * rhs_kj5;
                            res(i+1,j+6) += lhs_ik1 * rhs_kj6;
                            res(i+1,j+7) += lhs_ik1 * rhs_kj7;

                            res(i+2,j)   += lhs_ik2 * rhs_kj0;
                            res(i+2,j+1) += lhs_ik2 * rhs_kj1;
                            res(i+2,j+2) += lhs_ik2 * rhs_kj2;
                            res(i+2,j+3) += lhs_ik2 * rhs_kj3;
                            res(i+2,j+4) += lhs_ik2 * rhs_kj4;
                            res(i+2,j+5) += lhs_ik2 * rhs_kj5;
                            res(i+2,j+6) += lhs_ik2 * rhs_kj6;
                            res(i+2,j+7) += lhs_ik2 * rhs_kj7;

                            res(i+3,j)   += lhs_ik3 * rhs_kj0;
                            res(i+3,j+1) += lhs_ik3 * rhs_kj1;
                            res(i+3,j+2) += lhs_ik3 * rhs_kj2;
                            res(i+3,j+3) += lhs_ik3 * rhs_kj3;
                            res(i+3,j+4) += lhs_ik3 * rhs_kj4;
                            res(i+3,j+5) += lhs_ik3 * rhs_kj5;
                            res(i+3,j+6) += lhs_ik3 * rhs_kj6;
                            res(i+3,j+7) += lhs_ik3 * rhs_kj7;

                            res(i+4,j)   += lhs_ik4 * rhs_kj0;
                            res(i+4,j+1) += lhs_ik4 * rhs_kj1;
                            res(i+4,j+2) += lhs_ik4 * rhs_kj2;
                            res(i+4,j+3) += lhs_ik4 * rhs_kj3;
                            res(i+4,j+4) += lhs_ik4 * rhs_kj4;
                            res(i+4,j+5) += lhs_ik4 * rhs_kj5;
                            res(i+4,j+6) += lhs_ik4 * rhs_kj6;
                            res(i+4,j+7) += lhs_ik4 * rhs_kj7;

                            res(i+5,j)   += lhs_ik5 * rhs_kj0;
                            res(i+5,j+1) += lhs_ik5 * rhs_kj1;
                            res(i+5,j+2) += lhs_ik5 * rhs_kj2;
                            res(i+5,j+3) += lhs_ik5 * rhs_kj3;
                            res(i+5,j+4) += lhs_ik5 * rhs_kj4;
                            res(i+5,j+5) += lhs_ik5 * rhs_kj5;
                            res(i+5,j+6) += lhs_ik5 * rhs_kj6;
                            res(i+5,j+7) += lhs_ik5 * rhs_kj7;

                            res(i+6,j)   += lhs_ik6 * rhs_kj0;
                            res(i+6,j+1) += lhs_ik6 * rhs_kj1;
                            res(i+6,j+2) += lhs_ik6 * rhs_kj2;
                            res(i+6,j+3) += lhs_ik6 * rhs_kj3;
                            res(i+6,j+4) += lhs_ik6 * rhs_kj4;
                            res(i+6,j+5) += lhs_ik6 * rhs_kj5;
                            res(i+6,j+6) += lhs_ik6 * rhs_kj6;
                            res(i+6,j+7) += lhs_ik6 * rhs_kj7;

                            res(i+7,j)   += lhs_ik7 * rhs_kj0;
                            res(i+7,j+1) += lhs_ik7 * rhs_kj1;
                            res(i+7,j+2) += lhs_ik7 * rhs_kj2;
                            res(i+7,j+3) += lhs_ik7 * rhs_kj3;
                            res(i+7,j+4) += lhs_ik7 * rhs_kj4;
                            res(i+7,j+5) += lhs_ik7 * rhs_kj5;
                            res(i+7,j+6) += lhs_ik7 * rhs_kj6;
                            res(i+7,j+7) += lhs_ik7 * rhs_kj7;
                        } else {
                            // Handle remaining columns
                            for (int jj = j; jj < jmax; ++jj) {
                                const double rhs_kjj = rhs(k, jj);
                                res(i,jj)   += lhs_ik0 * rhs_kjj;
                                res(i+1,jj) += lhs_ik1 * rhs_kjj;
                                res(i+2,jj) += lhs_ik2 * rhs_kjj;
                                res(i+3,jj) += lhs_ik3 * rhs_kjj;
                                res(i+4,jj) += lhs_ik4 * rhs_kjj;
                                res(i+5,jj) += lhs_ik5 * rhs_kjj;
                                res(i+6,jj) += lhs_ik6 * rhs_kjj;
                                res(i+7,jj) += lhs_ik7 * rhs_kjj;
                            }
                        }
                    }
                }
            } else {
                // Handle remaining rows
                for (int ii = i; ii < imax; ++ii) {
                    for (int k = k0; k < kmax; ++k) {
                        const double lhs_ik = lhs(ii, k);
                        for (int j = j0; j < jmax; ++j) {
                            res(ii,j) += lhs_ik * rhs(k,j);
                        }
                    }
                }
            }
        }
    }
}

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) 
{
    const int m = lhs.rows();
    const int n = rhs.columns();
    const int p = lhs.columns();
    
    if (p != rhs.rows())
        throw std::runtime_error{"Matrix dimensions mismatch"};

    SimpleMatrix res{m, n};

    // Initialize result matrix to zero
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            res(i,j) = 0;
        }
    }

    // Block multiplication
    for (int i0 = 0; i0 < m; i0 += BLOCK_SIZE_M) {
        for (int k0 = 0; k0 < p; k0 += BLOCK_SIZE_K) {
            for (int j0 = 0; j0 < n; j0 += BLOCK_SIZE_N) {
                multiply_block<BLOCK_SIZE_M, BLOCK_SIZE_N, BLOCK_SIZE_K>(
                    lhs, rhs, res, i0, j0, k0);
            }
        }
    }

    return res;
}
