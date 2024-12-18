
#include <stdexcept>
#include "simplematrix.h"

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

  for (int i = 0; i < lhs_rows; ++i)
  {
    for (int j = 0; j < rhs_cols; ++j)
    {
      ValueType sum = ValueType(0);

      // Cache-friendly inner loop: iterate to minimize cache miss
      for (int k = 0; k < lhs_cols; ++k)
      {
        sum += lhs(i, k) * rhs(k, j);
      }

      res(i, j) = sum;
    }
  }

  return res;
}
