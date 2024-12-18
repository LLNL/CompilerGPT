
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    const int lhsRows = lhs.rows();
    const int lhsCols = lhs.columns();
    const int rhsCols = rhs.columns();

    if (lhsCols != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    SimpleMatrix res{lhsRows, rhsCols};
    using ValueType = SimpleMatrix::value_type;
    
    constexpr int blockSize = 32; // Tune based on hardware specifics
    auto min = [](int a, int b) { return a < b ? a : b; };

    // Triple loop of blocks to ensure the best data locality
    for (int ii = 0; ii < lhsRows; ii += blockSize) {
        for (int jj = 0; jj < rhsCols; jj += blockSize) {
            for (int kk = 0; kk < lhsCols; kk += blockSize) {
                for (int i = ii; i < min(ii + blockSize, lhsRows); ++i) {
                    for (int k = kk; k < min(kk + blockSize, lhsCols); ++k) {
                        const ValueType lhsValue = lhs(i, k);
                        int j = jj;

                        // Unroll the loop more aggressively
                        for (; j + 8 <= min(jj + blockSize, rhsCols); j += 8) {
                            res(i, j) += lhsValue * rhs(k, j);
                            res(i, j + 1) += lhsValue * rhs(k, j + 1);
                            res(i, j + 2) += lhsValue * rhs(k, j + 2);
                            res(i, j + 3) += lhsValue * rhs(k, j + 3);
                            res(i, j + 4) += lhsValue * rhs(k, j + 4);
                            res(i, j + 5) += lhsValue * rhs(k, j + 5);
                            res(i, j + 6) += lhsValue * rhs(k, j + 6);
                            res(i, j + 7) += lhsValue * rhs(k, j + 7);
                        }

                        // Process any remaining columns
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
