
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();

  if (lhsCols != rhs.rows()) // The condition remains unchanged
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  SimpleMatrix res{lhsRows, rhsCols};

  for (int i = 0; i < lhsRows; ++i)
  {
    for (int j = 0; j < rhsCols; ++j)
    {
      // Initialize element at (i, j) to 0.
      double sum = 0; // Use double or the appropriate type for floating-point precision

      for (int k = 0; k < lhsCols; ++k)
      {
        sum += lhs(i, k) * rhs(k, j);
      }

      res(i, j) = sum;
    }
  }

  return res;
}
