
#include <stdexcept>
#include "simplematrix.h"
#include <algorithm> // for std::min

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    const int lhsRows = lhs.rows();
    const int lhsCols = lhs.columns();
    const int rhsCols = rhs.columns();

    if (lhsCols != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    SimpleMatrix res{lhsRows, rhsCols};
    using ValueType = SimpleMatrix::value_type;

    const int blockSize = 32; // Tuned for specific CPU cache
    auto min = [](int a, int b) { return a < b ? a : b; };

    for (int ii = 0; ii < lhsRows; ii += blockSize) {
        for (int jj = 0; jj < rhsCols; jj += blockSize) {
            for (int kk = 0; kk < lhsCols; kk += blockSize) {
                for (int i = ii; i < min(ii + blockSize, lhsRows); ++i) {
                    for (int k = kk; k < min(kk + blockSize, lhsCols); ++k) {
                        const ValueType lhsValue = lhs(i, k);

                        // Unrolling the inner loop for potential performance gains
                        int j;
                        for (j = jj; j + 4 <= min(jj + blockSize, rhsCols); j += 4) {
                            res(i, j) += lhsValue * rhs(k, j);
                            res(i, j + 1) += lhsValue * rhs(k, j + 1);
                            res(i, j + 2) += lhsValue * rhs(k, j + 2);
                            res(i, j + 3) += lhsValue * rhs(k, j + 3);
                        }

                        // Handle any remaining elements
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
