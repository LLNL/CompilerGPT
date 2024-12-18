// $(CXX) -Wall -Wextra -O3 -march=native -DNDEBUG=1 perf.cc simplemtraix.cc -o perf

#include <iostream>
#include <chrono>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <limits>

#include "simplematrix.h"


void init(SimpleMatrix& mat)
{
  for (int i = 0; i < mat.rows(); ++i)
    for (int j = 0; j < mat.columns(); ++j)
      mat(i,j) = 1 + i * mat.columns() + j;
}

SimpleMatrix mul(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
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

void fail(const char* msg)
{
  throw std::runtime_error(msg);
}

void assert_equal(int lhs, int rhs, const char* msg)
{
  if (lhs != rhs)
    throw std::runtime_error(msg);
}

void assert_equal(SimpleMatrix::value_type lhs, SimpleMatrix::value_type rhs, const char* msg)
{
  if (std::fabs(lhs-rhs) > 0.1)
    throw std::runtime_error(msg);
}

void assert_equal_size(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  assert_equal(lhs.columns(), rhs.columns(), "number of columns is incorrect");
  assert_equal(lhs.rows(),    rhs.rows(),    "number of rows is incorrect");
}



void assert_equal(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  assert_equal_size(lhs, rhs);

  for (int i = 0; i < lhs.rows(); ++i)
    for (int j = 0; j < lhs.columns(); ++j)
      assert_equal(lhs(i,j), rhs(i,j), "element is incorrect");
}

void assert_datatype_size()
{
  SimpleMatrix lhs{1, 1};
  SimpleMatrix rhs{1, 1};

  lhs(0,0) = rhs(0,0) = std::numeric_limits<double>::max();

  SimpleMatrix res = lhs* rhs;
  SimpleMatrix exp = mul(lhs, rhs);

  assert_equal_size(res, exp);
  assert_equal(res(0,0), exp(0,0), "datatype too short; use SimpleMatrix::value_type");
}

void assert_size_check()
{
  SimpleMatrix lhs{3, 3};
  SimpleMatrix rhs{2, 2};

  try
  {
    SimpleMatrix res = lhs * rhs;

    fail("matrix dimension check failed. throw a std::runtime_error when dimensions do not match.");
  }
  catch (const std::runtime_error&)
  {
  }
}


int main()
{
  using time_point = std::chrono::time_point<std::chrono::system_clock>;

  assert_datatype_size();

  SimpleMatrix lhs{1483, 881};
  SimpleMatrix rhs{881, 1223};

  init(lhs); init(rhs);

  time_point   starttime = std::chrono::system_clock::now();
  SimpleMatrix res = lhs*rhs;
  time_point   endtime = std::chrono::system_clock::now();
  int          elapsedtime = std::chrono::duration_cast<std::chrono::milliseconds>(endtime-starttime).count();

  SimpleMatrix exp = mul(lhs,rhs);

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


