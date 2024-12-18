
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

    // Constants for block size - tune these based on your CPU's cache size
    constexpr int BLOCK_SIZE_M = 32;
    constexpr int BLOCK_SIZE_N = 32;
    constexpr int BLOCK_SIZE_K = 32;

    // Pre-initialize result matrix to zero using larger chunks
    for (int i = 0; i < m; i += 4) {
        for (int j = 0; j < n; j += 4) {
            // Unrolled initialization
            if (i + 3 < m && j + 3 < n) {
                res(i,j) = 0.0; res(i,j+1) = 0.0; res(i,j+2) = 0.0; res(i,j+3) = 0.0;
                res(i+1,j) = 0.0; res(i+1,j+1) = 0.0; res(i+1,j+2) = 0.0; res(i+1,j+3) = 0.0;
                res(i+2,j) = 0.0; res(i+2,j+1) = 0.0; res(i+2,j+2) = 0.0; res(i+2,j+3) = 0.0;
                res(i+3,j) = 0.0; res(i+3,j+1) = 0.0; res(i+3,j+2) = 0.0; res(i+3,j+3) = 0.0;
            } else {
                // Handle remaining elements
                for (int ii = i; ii < std::min(i + 4, m); ++ii) {
                    for (int jj = j; jj < std::min(j + 4, n); ++jj) {
                        res(ii,jj) = 0.0;
                    }
                }
            }
        }
    }

    // Block multiplication
    for (int i0 = 0; i0 < m; i0 += BLOCK_SIZE_M) {
        const int imax = std::min(i0 + BLOCK_SIZE_M, m);
        for (int k0 = 0; k0 < p; k0 += BLOCK_SIZE_K) {
            const int kmax = std::min(k0 + BLOCK_SIZE_K, p);
            for (int j0 = 0; j0 < n; j0 += BLOCK_SIZE_N) {
                const int jmax = std::min(j0 + BLOCK_SIZE_N, n);

                // Process blocks with loop unrolling
                for (int i = i0; i < imax; i += 4) {
                    for (int k = k0; k < kmax; ++k) {
                        if (i + 3 < imax) {
                            const double lhs_ik0 = lhs(i,k);
                            const double lhs_ik1 = lhs(i+1,k);
                            const double lhs_ik2 = lhs(i+2,k);
                            const double lhs_ik3 = lhs(i+3,k);

                            for (int j = j0; j < jmax; j += 4) {
                                if (j + 3 < jmax) {
                                    // Unrolled multiplication for 4x4 block
                                    res(i,j) += lhs_ik0 * rhs(k,j);
                                    res(i,j+1) += lhs_ik0 * rhs(k,j+1);
                                    res(i,j+2) += lhs_ik0 * rhs(k,j+2);
                                    res(i,j+3) += lhs_ik0 * rhs(k,j+3);

                                    res(i+1,j) += lhs_ik1 * rhs(k,j);
                                    res(i+1,j+1) += lhs_ik1 * rhs(k,j+1);
                                    res(i+1,j+2) += lhs_ik1 * rhs(k,j+2);
                                    res(i+1,j+3) += lhs_ik1 * rhs(k,j+3);

                                    res(i+2,j) += lhs_ik2 * rhs(k,j);
                                    res(i+2,j+1) += lhs_ik2 * rhs(k,j+1);
                                    res(i+2,j+2) += lhs_ik2 * rhs(k,j+2);
                                    res(i+2,j+3) += lhs_ik2 * rhs(k,j+3);

                                    res(i+3,j) += lhs_ik3 * rhs(k,j);
                                    res(i+3,j+1) += lhs_ik3 * rhs(k,j+1);
                                    res(i+3,j+2) += lhs_ik3 * rhs(k,j+2);
                                    res(i+3,j+3) += lhs_ik3 * rhs(k,j+3);
                                } else {
                                    // Handle remaining columns
                                    for (int jj = j; jj < jmax; ++jj) {
                                        res(i,jj) += lhs_ik0 * rhs(k,jj);
                                        res(i+1,jj) += lhs_ik1 * rhs(k,jj);
                                        res(i+2,jj) += lhs_ik2 * rhs(k,jj);
                                        res(i+3,jj) += lhs_ik3 * rhs(k,jj);
                                    }
                                }
                            }
                        } else {
                            // Handle remaining rows
                            for (int ii = i; ii < imax; ++ii) {
                                const double lhs_ik = lhs(ii,k);
                                for (int j = j0; j < jmax; ++j) {
                                    res(ii,j) += lhs_ik * rhs(k,j);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return res;
}
