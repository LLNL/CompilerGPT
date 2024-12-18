
#include <stdexcept>
#include <algorithm> // For std::min

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

    // New implementation detail starts here
    constexpr int blockSize = 32; // Larger block size based on cache line size
    
    for (int i_block = 0; i_block < lhs_rows; i_block += blockSize) {
        for (int j_block = 0; j_block < rhs_columns; j_block += blockSize) {
            for (int k_block = 0; k_block < lhs_columns; k_block += blockSize) {
                for (int i = i_block; i < std::min(i_block + blockSize, lhs_rows); ++i) {
                    for (int j = j_block; j < std::min(j_block + blockSize, rhs_columns); ++j) {
                        ValueType sum = res(i, j);
                        for (int k = k_block; k < std::min(k_block + blockSize, lhs_columns); ++k) {
                            // Access the lhs and rhs matrices directly if possible
                            sum += lhs(i, k) * rhs(k, j);
                        }
                        res(i, j) = sum;
                    }
                }
            }
        }
    }

    return res;
}
