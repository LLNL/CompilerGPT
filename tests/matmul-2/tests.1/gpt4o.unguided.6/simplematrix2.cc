
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();
  
  using ValueType = SimpleMatrix::value_type; // Define the appropriate type for matrix values

  SimpleMatrix res{lhsRows, rhsCols};

  for (int i = 0; i < lhsRows; ++i)
  {
    for (int j = 0; j < rhsCols; ++j)
    {
      ValueType sum = ValueType();

      for (int k = 0; k < lhsCols; ++k)
      {
        sum += lhs(i, k) * rhs(k, j);
      }

      res(i, j) = sum;
    }
  }

  return res;
}
