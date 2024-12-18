
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
    
    for (int i = 0; i < lhsRows; ++i) {
        for (int k = 0; k < lhsCols; ++k) {
            const ValueType lhsValue = lhs(i, k);
            for (int j = 0; j < rhsCols; ++j) {
                res(i, j) += lhsValue * rhs(k, j);
            }
        }
    }

    return res;
}
