// $(CXX) -Wall -Wextra -O3 -march=native -DNDEBUG=1 perf.cc simplemtraix.cc -o perf

#include <iostream>
#include <chrono>
#include <cmath>
#include <memory>
#include <stdexcept>

struct SimpleMatrix
{
    using value_type = long double;

    SimpleMatrix(int rows, int cols)
    : m(rows), n(cols), mem(new value_type[rows*cols])
    {}

    value_type operator()(int row, int col) const
    {
      return mem[row*columns() + col];
    }

    value_type& operator()(int row, int col)
    {
      return mem[row*columns() + col];
    }

    int rows()    const { return m; }
    int columns() const { return n; }

  private:
    int                           m;
    int                           n;
    std::unique_ptr<value_type[]> mem;
};


SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs);


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

void assert_equal(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  assert_equal(lhs.columns(), rhs.columns(), "columns()");
  assert_equal(lhs.rows(),    rhs.rows(), "columns()");

  for (int i = 0; i < lhs.rows(); ++i)
    for (int j = 0; j < lhs.columns(); ++j)
      assert_equal(lhs(i,j), rhs(i,j), "element");
}


int main()
{
  typedef std::chrono::time_point<std::chrono::system_clock> time_point;

  SimpleMatrix lhs{1423, 888};
  SimpleMatrix rhs{888, 1234};

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
  catch (...)
  {
    exit(1);
  }

  std::cout << elapsedtime << std::endl;
  return 0;
}


