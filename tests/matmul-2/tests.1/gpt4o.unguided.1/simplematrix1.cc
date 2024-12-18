
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();
  
  if (lhsCols != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  SimpleMatrix res{lhsRows, rhsCols};

  for (int i = 0; i < lhsRows; ++i)
  {
    for (int j = 0; j < rhsCols; ++j)
    {
      double sum = 0.0;  // Use a temporary variable for summation

      for (int k = 0; k < lhsCols; ++k)
        sum += lhs(i, k) * rhs(k, j);

      res(i, j) = sum;
    }
  }

  return res;
}
