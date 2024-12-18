
#include <stdexcept>
#include "simplematrix.h"

// Define block size for the blocking technique with further potential tuning
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

                        int k = kk;
                        int kEnd = std::min(kk + BLOCK_SIZE, lhs_cols);

                        // Optimize by unrolling the loop
                        for (; k + 7 < kEnd; k += 8) {
                            sum += lhs(i, k) * rhs(k, j);
                            sum += lhs(i, k + 1) * rhs(k + 1, j);
                            sum += lhs(i, k + 2) * rhs(k + 2, j);
                            sum += lhs(i, k + 3) * rhs(k + 3, j);
                            sum += lhs(i, k + 4) * rhs(k + 4, j);
                            sum += lhs(i, k + 5) * rhs(k + 5, j);
                            sum += lhs(i, k + 6) * rhs(k + 6, j);
                            sum += lhs(i, k + 7) * rhs(k + 7, j);
                        }

                        // Handle remaining elements
                        for (; k < kEnd; ++k) {
                            sum += lhs(i, k) * rhs(k, j);
                        }

                        res(i, j) = sum;  // Store the computed sum back
                    }
                }
            }
        }
    }

    return res;
}
