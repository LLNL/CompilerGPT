
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

// Set block size taking into account cache-line size and system architecture
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
                int iMax = std::min(i + BLOCK_SIZE, lhsRows);
                int jMax = std::min(j + BLOCK_SIZE, rhsCols);
                int kMax = std::min(k + BLOCK_SIZE, lhsCols);

                // Further unroll innermost loop for potential multiplier of unroll
                for (int ib = i; ib < iMax; ++ib)
                {
                    for (int jb = j; jb < jMax; ++jb)
                    {
                        SimpleMatrix::value_type sum = res(ib, jb);  // Accumulate into result position

                        int kb;
                        for (kb = k; kb <= kMax - 8; kb += 8)
                        {
                            // Unroll by 8 for better instruction throughput
                            sum += lhs(ib, kb) * rhs(kb, jb);
                            sum += lhs(ib, kb + 1) * rhs(kb + 1, jb);
                            sum += lhs(ib, kb + 2) * rhs(kb + 2, jb);
                            sum += lhs(ib, kb + 3) * rhs(kb + 3, jb);
                            sum += lhs(ib, kb + 4) * rhs(kb + 4, jb);
                            sum += lhs(ib, kb + 5) * rhs(kb + 5, jb);
                            sum += lhs(ib, kb + 6) * rhs(kb + 6, jb);
                            sum += lhs(ib, kb + 7) * rhs(kb + 7, jb);
                        }
                      
                        // Handle remaining elements
                        for (; kb < kMax; ++kb)
                        {
                            sum += lhs(ib, kb) * rhs(kb, jb);
                        }

                        res(ib, jb) = sum;  // Write after accumulation to minimize writes
                    }
                }
            }
        }
    }

    return res;
}
