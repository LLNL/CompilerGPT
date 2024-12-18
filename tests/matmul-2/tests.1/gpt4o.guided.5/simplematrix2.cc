
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  // Ensure dimensions are compatible for matrix multiplication
  if (lhs.columns() != rhs.rows()) {
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};
  }

  // Create result matrix object with appropriate dimensions
  SimpleMatrix res(lhs.rows(), rhs.columns());
  
  // Cache dimensions to avoid method calls within the loop
  const int lhsRows = lhs.rows();
  const int rhsColumns = rhs.columns();
  const int sharedDim = lhs.columns();

  // Loop through each element of the result matrix
  for (int i = 0; i < lhsRows; ++i) {
    for (int j = 0; j < rhsColumns; ++j) {
      
      // Declare the sum variable using SimpleMatrix::value_type
      SimpleMatrix::value_type sum = 0;

      for (int k = 0; k < sharedDim; ++k) {
        // Accumulate the product of the corresponding elements
        sum += lhs(i, k) * rhs(k, j);
      }

      // Assign the computed sum to the result matrix
      res(i, j) = sum;
    }
  }

  return res;
}
