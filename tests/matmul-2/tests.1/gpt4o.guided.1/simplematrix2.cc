
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  // Ensure the matrices can be multiplied
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  // Retrieve dimensions once
  int leftRows = lhs.rows();
  int leftColumns = lhs.columns();
  int rightColumns = rhs.columns();
  
  // Create the resulting matrix
  SimpleMatrix res{leftRows, rightColumns};

  // Perform matrix multiplication
  for (int i = 0; i < leftRows; ++i)
  {
    for (int j = 0; j < rightColumns; ++j)
    {
      SimpleMatrix::value_type sum = static_cast<SimpleMatrix::value_type>(0); // Ensure using the correct datatype

      for (int k = 0; k < leftColumns; ++k)
      {
        // Compute the dot product using the appropriate type
        sum += lhs(i, k) * rhs(k, j);
      }
      res(i, j) = sum;
    }
  }

  // Return the result
  return res;
}
