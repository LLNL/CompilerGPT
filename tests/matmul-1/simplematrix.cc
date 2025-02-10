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


