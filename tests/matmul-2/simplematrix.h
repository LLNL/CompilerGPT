#include <memory>

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


