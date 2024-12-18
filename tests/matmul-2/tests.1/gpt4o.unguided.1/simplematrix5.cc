
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

// Iterate over sub-matrices with tuning for cache, aimed size for cache affinity.
constexpr int BLOCK_SIZE = 32;

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
    int lhsRows = lhs.rows();
    int lhsCols = lhs.columns();
    int rhsCols = rhs.columns();

    if (lhsCols != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    SimpleMatrix res{lhsRows, rhsCols};

    for (int i = 0; i < lhsRows; i += BLOCK_SIZE)
    {
        for (int j = 0; j < rhsCols; j += BLOCK_SIZE)
        {
            for (int k = 0; k < lhsCols; k += BLOCK_SIZE)
            {
                // Compute block boundaries
                int iMax = std::min(i + BLOCK_SIZE, lhsRows);
                int jMax = std::min(j + BLOCK_SIZE, rhsCols);
                int kMax = std::min(k + BLOCK_SIZE, lhsCols);

                // Block computation with manual unrolling
                for (int ib = i; ib < iMax; ++ib)
                {
                    for (int jb = j; jb < jMax; ++jb)
                    {
                        SimpleMatrix::value_type sum = 0;

                        int kb;
                        for (kb = k; kb <= kMax - 4; kb += 4)
                        {
                            // Unrolling the innermost loop to reduce loop control overhead
                            sum += lhs(ib, kb) * rhs(kb, jb);
                            sum += lhs(ib, kb + 1) * rhs(kb + 1, jb);
                            sum += lhs(ib, kb + 2) * rhs(kb + 2, jb);
                            sum += lhs(ib, kb + 3) * rhs(kb + 3, jb);
                        }

                        // Handle any remaining elements
                        for (; kb < kMax; ++kb)
                        {
                            sum += lhs(ib, kb) * rhs(kb, jb);
                        }

                        res(ib, jb) += sum;
                    }
                }
            }
        }
    }

    return res;
}
