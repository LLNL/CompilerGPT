
#include <stdexcept>
#include "simplematrix.h"

// Define block size for blocking technique
constexpr int BLOCK_SIZE = 16;

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
    const int lhs_cols = lhs.columns();
    const int lhs_rows = lhs.rows();
    const int rhs_cols = rhs.columns();
    const int rhs_rows = rhs.rows();

    if (lhs_cols != rhs_rows) 
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    using ValueType = SimpleMatrix::value_type;
    SimpleMatrix res{lhs_rows, rhs_cols};

    for (int ii = 0; ii < lhs_rows; ii += BLOCK_SIZE)
    {
        for (int jj = 0; jj < rhs_cols; jj += BLOCK_SIZE)
        {
            for (int kk = 0; kk < lhs_cols; kk += BLOCK_SIZE)
            {
                for (int i = ii; i < std::min(ii + BLOCK_SIZE, lhs_rows); ++i)
                {
                    for (int j = jj; j < std::min(jj + BLOCK_SIZE, rhs_cols); ++j)
                    {
                        ValueType sum = res(i, j);
                        const ValueType* lhs_row_ptr = &lhs(i, 0);  // Pointer to the start of the lhs row
                        const ValueType* rhs_col_ptr = &rhs(0, j);  // Pointer to the start of the rhs column

                        int k = kk;
                        int kEnd = std::min(kk + BLOCK_SIZE, lhs_cols);

                        // Manually unroll and use pointer arithmetic
                        for (; k + 7 < kEnd; k += 8) {
                            sum += lhs_row_ptr[k] * rhs_col_ptr[k * rhs_cols];
                            sum += lhs_row_ptr[k + 1] * rhs_col_ptr[(k + 1) * rhs_cols];
                            sum += lhs_row_ptr[k + 2] * rhs_col_ptr[(k + 2) * rhs_cols];
                            sum += lhs_row_ptr[k + 3] * rhs_col_ptr[(k + 3) * rhs_cols];
                            sum += lhs_row_ptr[k + 4] * rhs_col_ptr[(k + 4) * rhs_cols];
                            sum += lhs_row_ptr[k + 5] * rhs_col_ptr[(k + 5) * rhs_cols];
                            sum += lhs_row_ptr[k + 6] * rhs_col_ptr[(k + 6) * rhs_cols];
                            sum += lhs_row_ptr[k + 7] * rhs_col_ptr[(k + 7) * rhs_cols];
                        }
                        // Handle remaining elements
                        for (; k < kEnd; ++k) {
                            sum += lhs_row_ptr[k] * rhs_col_ptr[k * rhs_cols];
                        }

                        res(i, j) = sum;  // Store the computed sum
                    }
                }
            }
        }
    }

    return res;
}
