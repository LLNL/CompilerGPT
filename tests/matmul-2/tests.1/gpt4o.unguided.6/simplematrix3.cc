
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    const int lhsRows = lhs.rows();
    const int lhsCols = lhs.columns();
    const int rhsCols = rhs.columns();

    // Ensure that the condition for matrix multiplication is met
    if (lhsCols != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    SimpleMatrix res{lhsRows, rhsCols};

    // Use lhs internal mechanism for element type inference
    using ValueType = SimpleMatrix::value_type;
  
    for (int i = 0; i < lhsRows; ++i) {
        for (int j = 0; j < rhsCols; ++j) {
            ValueType sum = ValueType();

            // Refactor the innermost computation to increase clarity and potentially aid in compiler optimization
            for (int k = 0; k < lhsCols; ++k) {
                sum += lhs(i, k) * rhs(k, j);
            }

            res(i, j) = sum;
        }
    }

    return res;
}
