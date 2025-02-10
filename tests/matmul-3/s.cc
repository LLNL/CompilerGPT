#include <stdexcept>

#include "s.h"

S compute(const S& lhs, const S& rhs)
{
  if (lhs.xlen() != rhs.ylen())
    throw std::runtime_error{"mismatch"};

  S res{lhs.ylen(), rhs.xlen()};

  for (int i = 0; i < res.ylen(); ++i)
  {
    for (int j = 0; j < res.xlen(); ++j)
    {
      res(i,j) = 0;

      for (int k = 0; k < lhs.xlen(); ++k)
        res(i,j) += lhs(i, k) * rhs(k, j);
    }
  }

  return res;
}

