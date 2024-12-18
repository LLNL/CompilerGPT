
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

// Defining block size specifically to improve cache usage
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

                for (int ib = i; ib < iMax; ++ib)
                {
                    for (int jb = j; jb < jMax; ++jb)
                    {
                        SimpleMatrix::value_type sum = res(ib, jb);

                        for (int kb = k; kb < kMax; ++kb)
                        {
                            // Perform multiplication and accumulation
                            sum += lhs(ib, kb) * rhs(kb, jb);
                        }

                        res(ib, jb) = sum;
                    }
                }
            }
        }
    }

    return res;
}
