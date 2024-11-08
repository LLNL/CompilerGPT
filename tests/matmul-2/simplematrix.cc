#include <stdexcept>

#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  SimpleMatrix res{lhs.rows(), rhs.columns()};

  for (int i = 0; i < res.rows(); ++i)
  {
    for (int j = 0; j < res.columns(); ++j)
    {
      res(i,j) = 0;

      for (int k = 0; k < lhs.columns(); ++k)
        res(i,j) += lhs(i, k) * rhs(k, j);
    }
  }

  return res;
}


