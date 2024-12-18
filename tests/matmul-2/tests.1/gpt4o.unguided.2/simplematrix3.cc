
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    using ValueType = SimpleMatrix::value_type;

    int lhsRows = lhs.rows();
    int lhsCols = lhs.columns();
    int rhsCols = rhs.columns();
    
    if (lhsCols != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    SimpleMatrix res{lhsRows, rhsCols};

    // Create a transpose of the rhs matrix to improve cache locality
    SimpleMatrix rhsTransposed{rhsCols, lhsCols};
    for (int i = 0; i < lhsCols; ++i) {
        for (int j = 0; j < rhsCols; ++j) {
            rhsTransposed(j, i) = rhs(i, j);
        }
    }

    for (int i = 0; i < lhsRows; ++i) {
        for (int j = 0; j < rhsCols; ++j) {
            ValueType sum = ValueType{};
            int k = 0;
            // Loop unrolling can be applied to decrease the loop iteration overhead.
            for (; k + 3 < lhsCols; k += 4) {
                sum += lhs(i, k) * rhsTransposed(j, k);
                sum += lhs(i, k + 1) * rhsTransposed(j, k + 1);
                sum += lhs(i, k + 2) * rhsTransposed(j, k + 2);
                sum += lhs(i, k + 3) * rhsTransposed(j, k + 3);
            }
            // Handle remaining elements
            for (; k < lhsCols; ++k) {
                sum += lhs(i, k) * rhsTransposed(j, k);
            }
            res(i, j) = sum;
        }
    }

    return res;
}
