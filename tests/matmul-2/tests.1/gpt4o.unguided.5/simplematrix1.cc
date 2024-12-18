
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  // Check for dimension compatibility
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  // Initialize the result matrix once
  SimpleMatrix res{lhs.rows(), rhs.columns()};
  
  // Pre-compute dimensions
  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();
  
  // Loop through matrices for multiplication
  for (int i = 0; i < lhsRows; ++i)
  {
    for (int k = 0; k < lhsCols; ++k) // Change in loop order for better cache use
    {
      for (int j = 0; j < rhsCols; ++j)
      {
        // Use existing initialized value of res(i,j), avoids redundant write
        res(i, j) += lhs(i, k) * rhs(k, j);
      }
    }
  }

  return res;
}
