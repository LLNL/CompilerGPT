
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix &lhs, const SimpleMatrix &rhs) {
    // Ensure matrices can be multiplied
    if (lhs.columns() != rhs.rows()) {
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};
    }

    using ValueType = SimpleMatrix::value_type;

    int lhsRows = lhs.rows();
    int lhsCols = lhs.columns();
    int rhsCols = rhs.columns();

    SimpleMatrix res{lhsRows, rhsCols};

    // Transpose the rhs matrix for better memory access patterns
    SimpleMatrix rhsTransposed(rhsCols, lhsCols);
    for (int i = 0; i < lhsCols; ++i) {
        for (int j = 0; j < rhsCols; ++j) {
            rhsTransposed(j, i) = rhs(i, j); 
        }
    }

    // Standard matrix multiplication without SIMD due to `long double` type
    for (int i = 0; i < lhsRows; ++i) {
        for (int j = 0; j < rhsCols; ++j) {
            ValueType sum = static_cast<ValueType>(0);
            for (int k = 0; k < lhsCols; ++k) {
                sum += lhs(i, k) * rhsTransposed(j, k);
            }
            res(i, j) = sum;
        }
    }

    return res;
}

