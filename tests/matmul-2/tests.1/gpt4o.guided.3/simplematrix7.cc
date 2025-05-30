
#include <stdexcept>
#include <algorithm> // for std::min

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

    constexpr int blockSize = 32; // Targeted for cache efficiency but should be profiled

    for (int ii = 0; ii < lhs_rows; ii += blockSize) {
        for (int kk = 0; kk < lhs_columns; kk += blockSize) {
            for (int jj = 0; jj < rhs_columns; jj += blockSize) {
                for (int i = ii; i < std::min(ii + blockSize, lhs_rows); ++i) {
                    for (int j = jj; j < std::min(jj + blockSize, rhs_columns); ++j) {
                        ValueType sum = res(i, j); // use initial value if res is initialized
                        // Prefetch rhs entries for better alignment possibilities
                        for (int k = kk; k < std::min(kk + blockSize, lhs_columns); ++k) {
                            ValueType lhs_val = lhs(i, k);
                            ValueType rhs_val = rhs(k, j);
                            sum += lhs_val * rhs_val;
                        }
                        res(i, j) = sum;
                    }
                }
            }
        }
    }

    return res;
}
