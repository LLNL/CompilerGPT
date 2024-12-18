
#include <stdexcept>

#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    using ValueType = SimpleMatrix::value_type;
    
    int lhs_rows = lhs.rows();
    int lhs_columns = lhs.columns();
    int rhs_columns = rhs.columns();

    if (lhs_columns != rhs.rows()) {
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};
    }

    SimpleMatrix res{lhs_rows, rhs_columns};

    for (int i = 0; i < lhs_rows; ++i) {
        for (int j = 0; j < rhs_columns; ++j) {
            ValueType sum = ValueType{};
            for (int k = 0; k < lhs_columns; ++k) {
                sum += lhs(i, k) * rhs(k, j);
            }
            res(i, j) = sum;
        }
    }

    return res;
}
