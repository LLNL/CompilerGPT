
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  // Check for dimension compatibility
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"Dimension mismatch: lhs.columns() != rhs.rows()"};

  // Create matrix for result
  SimpleMatrix res(lhs.rows(), rhs.columns());

  // Pre-compute matrix dimensions
  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();

  using ValueType = typename SimpleMatrix::value_type;

  // Intermediate buffer to encourage the compiler to reuse registers
  std::vector<ValueType> row_buffer(lhsCols);

  // Optimize using row buffering
  for (int i = 0; i < lhsRows; ++i)
  {
    // Load the entire row of lhs into buffering to improve cache efficiency
    for (int k = 0; k < lhsCols; ++k)
    {
      row_buffer[k] = lhs(i, k);
    }

    for (int j = 0; j < rhsCols; ++j)
    {
      ValueType sum = 0;

      for (int k = 0; k < lhsCols; ++k)
      {
        sum += row_buffer[k] * rhs(k, j);
      }

      res(i, j) = sum;
    }
  }

  return res;
}
