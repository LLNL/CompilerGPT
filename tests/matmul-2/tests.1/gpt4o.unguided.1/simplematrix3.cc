
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();
  
  if (lhsCols != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  SimpleMatrix res{lhsRows, rhsCols};

  for (int i = 0; i < lhsRows; ++i)
  {
    for (int j = 0; j < rhsCols; ++j)
    {
      SimpleMatrix::value_type sum = 0; 

      for (int k = 0; k < lhsCols; ++k)
      {
        // Utilize temporary variables to enhance potential register allocation
        auto lhs_val = lhs(i, k);
        auto rhs_val = rhs(k, j);
        sum += lhs_val * rhs_val;
      }

      res(i, j) = sum;
    }
  }

  return res;
}
