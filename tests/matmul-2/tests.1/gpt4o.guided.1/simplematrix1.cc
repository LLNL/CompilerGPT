
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  int leftRows = lhs.rows();
  int leftColumns = lhs.columns();
  int rightColumns = rhs.columns();
  
  SimpleMatrix res{leftRows, rightColumns};

  // Cache commonly accessed values
  for (int i = 0; i < leftRows; ++i)
  {
    for (int j = 0; j < rightColumns; ++j)
    {
      double sum = 0.0; // Use 'double' assuming our matrix works on double; adjust based on actual type

      for (int k = 0; k < leftColumns; ++k)
      {
        sum += lhs(i, k) * rhs(k, j);
      }
      res(i, j) = sum;
    }
  }

  return res;
}
