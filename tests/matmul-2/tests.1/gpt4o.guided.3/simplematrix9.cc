
#include <stdexcept>
#include <algorithm>

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

    constexpr int blockSize = 16;  // Consider adjusting to fit hardware better

    for (int ii = 0; ii < lhs_rows; ii += blockSize) {
        for (int kk = 0; kk < lhs_columns; kk += blockSize) {
            for (int jj = 0; jj < rhs_columns; jj += blockSize) {
                for (int i = ii; i < std::min(ii + blockSize, lhs_rows); ++i) {
                    for (int j = jj; j < std::min(jj + blockSize, rhs_columns); ++j) {
                        ValueType sum = static_cast<ValueType>(0);  // Ensuring correct type initialization
                        for (int k = kk; k < std::min(kk + blockSize, lhs_columns); ++k) {
                            sum += lhs(i, k) * rhs(k, j);
                        }
                        res(i, j) += sum;  // Accumulate the value to handle partial sums from blocks
                    }
                }
            }
        }
    }

    return res;
}
