
#include <stdexcept>
#include "simplematrix.h"  // Assuming SimpleMatrix and other necessary declarations are defined here

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  // Check for dimension compatibility
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"Dimension mismatch: lhs.columns() != rhs.rows()"};

  // Create a matrix to hold the result, results assumed to be zero-initialized
  SimpleMatrix res(lhs.rows(), rhs.columns());
  
  // Pre-compute dimensions
  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();

  // Using SimpleMatrix::value_type to ensure typing consistency
  using ValueType = typename SimpleMatrix::value_type;

  // Loop through matrices for multiplication
  for (int i = 0; i < lhsRows; ++i)
  {
    for (int j = 0; j < rhsCols; ++j)
    {
      ValueType sum = ValueType();  // Initialize with default constructor, should be zero

      for (int k = 0; k < lhsCols; ++k)
      {
        sum += lhs(i, k) * rhs(k, j);
      }
      res(i, j) = sum;
    }
  }

  return res;
}
