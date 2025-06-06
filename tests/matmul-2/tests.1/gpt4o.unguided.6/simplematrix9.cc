
#include <stdexcept>
#include "simplematrix.h"

// A directive to encourage the compiler to vectorize loops if possible
#if defined(__GNUC__) || defined(__clang__)
#define PRAGMA_VECTORIZE _Pragma("GCC ivdep")
#define PRAGMA_UNROLL_4 _Pragma("GCC unroll 4")
#endif

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    const int lhsRows = lhs.rows();
    const int lhsCols = lhs.columns();
    const int rhsCols = rhs.columns();

    if (lhsCols != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    SimpleMatrix res{lhsRows, rhsCols};
    using ValueType = SimpleMatrix::value_type;

    constexpr int blockSize = 32; // Continue to tune per architecture
    auto min = [](int a, int b) { return a < b ? a : b; };

    // Block-wise loop still maintained for cache efficiency
    for (int ii = 0; ii < lhsRows; ii += blockSize) {
        for (int jj = 0; jj < rhsCols; jj += blockSize) {
            for (int kk = 0; kk < lhsCols; kk += blockSize) {
                for (int i = ii; i < min(ii + blockSize, lhsRows); ++i) {
                    PRAGMA_VECTORIZE
                    for (int k = kk; k < min(kk + blockSize, lhsCols); ++k) {
                        const ValueType lhsValue = lhs(i, k);
                        int j = jj;

                        PRAGMA_UNROLL_4
                        for (; j + 16 <= min(jj + blockSize, rhsCols); j += 16) {
                            res(i, j) += lhsValue * rhs(k, j);
                            res(i, j + 1) += lhsValue * rhs(k, j + 1);
                            res(i, j + 2) += lhsValue * rhs(k, j + 2);
                            res(i, j + 3) += lhsValue * rhs(k, j + 3);
                            res(i, j + 4) += lhsValue * rhs(k, j + 4);
                            res(i, j + 5) += lhsValue * rhs(k, j + 5);
                            res(i, j + 6) += lhsValue * rhs(k, j + 6);
                            res(i, j + 7) += lhsValue * rhs(k, j + 7);
                            res(i, j + 8) += lhsValue * rhs(k, j + 8);
                            res(i, j + 9) += lhsValue * rhs(k, j + 9);
                            res(i, j + 10) += lhsValue * rhs(k, j + 10);
                            res(i, j + 11) += lhsValue * rhs(k, j + 11);
                            res(i, j + 12) += lhsValue * rhs(k, j + 12);
                            res(i, j + 13) += lhsValue * rhs(k, j + 13);
                            res(i, j + 14) += lhsValue * rhs(k, j + 14);
                            res(i, j + 15) += lhsValue * rhs(k, j + 15);
                        }

                        for (; j < min(jj + blockSize, rhsCols); ++j) {
                            res(i, j) += lhsValue * rhs(k, j);
                        }
                    }
                }
            }
        }
    }

    return res;
}
