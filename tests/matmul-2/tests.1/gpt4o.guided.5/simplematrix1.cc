
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  SimpleMatrix res{lhs.rows(), rhs.columns()};
  
  int rows = res.rows();
  int columns = res.columns();
  int shared_dim = lhs.columns();

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      // Initialize res(i, j) outside the innermost loop.
      double sum = 0.0;
      
      for (int k = 0; k < shared_dim; ++k) {
        // Access lhs(i, k) and rhs(k, j) inside the innermost loop
        sum += lhs(i, k) * rhs(k, j);
      }
      
      res(i, j) = sum;
    }
  }

  return res;
}
