
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

    const int blockSize = 32; // Can be tuned based on the architecture

    for (int ii = 0; ii < lhsRows; ii += blockSize) {
        for (int jj = 0; jj < rhsCols; jj += blockSize) {
            for (int kk = 0; kk < lhsCols; kk += blockSize) {
                for (int i = ii; i < std::min(ii + blockSize, lhsRows); ++i) {
                    for (int k = kk; k < std::min(kk + blockSize, lhsCols); ++k) {
                        const ValueType lhsValue = lhs(i, k);
                        for (int j = jj; j < std::min(jj + blockSize, rhsCols); ++j) {
                            res(i, j) += lhsValue * rhs(k, j);
                        }
                    }
                }
            }
        }
    }

    return res;
}
