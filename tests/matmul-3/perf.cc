// $(CXX) -Wall -Wextra -O3 -march=native -DNDEBUG=1 perf.cc simplemtraix.cc -o perf

#include <iostream>
#include <chrono>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <limits>

#include "s.h"


void init(S& mat)
{
  for (int i = 0; i < mat.ylen(); ++i)
    for (int j = 0; j < mat.xlen(); ++j)
      mat(i,j) = 1 + i * mat.xlen() + j;
}

S mul(const S& lhs, const S& rhs)
{
  if (lhs.xlen() != rhs.ylen())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

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

void fail(const char* msg)
{
  throw std::runtime_error(msg);
}

void assert_equal(int lhs, int rhs, const char* msg)
{
  if (lhs != rhs)
    throw std::runtime_error(msg);
}

void assert_equal(S::value_type lhs, S::value_type rhs, const char* msg)
{
  if (std::fabs(lhs-rhs) > 0.1)
    throw std::runtime_error(msg);
}

void assert_equal_size(const S& lhs, const S& rhs)
{
  assert_equal(lhs.xlen(), rhs.xlen(), "number of columns is incorrect");
  assert_equal(lhs.ylen(), rhs.ylen(), "number of rows is incorrect");
}

void assert_equal(const S& lhs, const S& rhs)
{
  assert_equal_size(lhs, rhs);

  for (int i = 0; i < lhs.ylen(); ++i)
    for (int j = 0; j < lhs.xlen(); ++j)
      assert_equal(lhs(i,j), rhs(i,j), "element is incorrect");
}

void assert_datatype_size()
{
  S lhs{1, 1};
  S rhs{1, 1};

  lhs(0,0) = rhs(0,0) = std::numeric_limits<double>::max();

  S res = compute(lhs, rhs);
  S exp = mul(lhs, rhs);

  assert_equal_size(res, exp);
  assert_equal(res(0,0), exp(0,0), "datatype too short; use S::value_type");
}

void assert_size_check()
{
  S lhs{3, 3};
  S rhs{2, 2};

  try
  {
    S res = compute(lhs, rhs);

    fail("matrix dimension check failed. throw a std::runtime_error when dimensions do not match.");
  }
  catch (const std::runtime_error&)
  {
  }
}


int main()
{
  using time_point = std::chrono::time_point<std::chrono::system_clock>;

  assert_size_check();
  assert_datatype_size();

  S lhs{1483, 881};
  S rhs{881, 1223};

  init(lhs); init(rhs);

  time_point starttime   = std::chrono::system_clock::now();
  S          res         = compute(lhs, rhs);
  time_point endtime     = std::chrono::system_clock::now();
  int        elapsedtime = std::chrono::duration_cast<std::chrono::milliseconds>(endtime-starttime).count();
  S          exp         = mul(lhs, rhs);

  try
  {
    assert_equal(res, exp);
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << std::endl;
    exit(1);
  }

  std::cout << elapsedtime << std::endl;
  return 0;
}


