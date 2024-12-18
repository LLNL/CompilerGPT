
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    using ValueType = SimpleMatrix::value_type; // Use the typedef from SimpleMatrix for the element type

    int lhsRows = lhs.rows();
    int lhsCols = lhs.columns();
    int rhsCols = rhs.columns();
    
    if (lhsCols != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    SimpleMatrix res{lhsRows, rhsCols};

    for (int i = 0; i < lhsRows; ++i) {
        for (int j = 0; j < rhsCols; ++j) {
            ValueType sum = ValueType{}; // Initialize sum using the default constructor of ValueType
            for (int k = 0; k < lhsCols; ++k) {
                sum += lhs(i, k) * rhs(k, j);
            }
            res(i, j) = sum;
        }
    }

    return res;
}
