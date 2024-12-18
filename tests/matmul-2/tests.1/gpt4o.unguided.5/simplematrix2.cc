
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  // Check for dimension compatibility
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  // Initialize the result matrix, assuming it's initially zeroed
  SimpleMatrix res{lhs.rows(), rhs.columns()};
  
  // Pre-compute dimensions
  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();
  
  // Loop through matrices for multiplication
  for (int i = 0; i < lhsRows; ++i)
  {
    for (int j = 0; j < rhsCols; ++j)
    {
      // Read lhs row once and res(i, j) to avoid multiple function calls
      double sum = 0.0;
      for (int k = 0; k < lhsCols; ++k)
      {
        sum += lhs(i, k) * rhs(k, j);
      }
      res(i, j) = sum;
    }
  }

  return res;
}
