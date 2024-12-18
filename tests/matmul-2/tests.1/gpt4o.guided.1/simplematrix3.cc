
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  int leftRows = lhs.rows();
  int leftColumns = lhs.columns();
  int rightColumns = rhs.columns();
  
  SimpleMatrix res{leftRows, rightColumns};

  for (int i = 0; i < leftRows; ++i)
  {
    for (int k = 0; k < leftColumns; ++k)
    {
      SimpleMatrix::value_type lhsVal = lhs(i, k);

      for (int j = 0; j < rightColumns; ++j)
      {
        res(i, j) += lhsVal * rhs(k, j);
      }
    }
  }

  return res;
}
