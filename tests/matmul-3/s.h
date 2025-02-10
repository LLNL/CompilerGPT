#include <memory>

struct S
{
    using value_type = long double;

    S(int rows, int cols)
    : m(rows), n(cols), mem(new value_type[rows*cols])
    {}

    value_type operator()(int row, int col) const
    {
      return mem[row*xlen() + col];
    }

    value_type& operator()(int row, int col)
    {
      return mem[row*xlen() + col];
    }

    int ylen() const { return m; }
    int xlen() const { return n; }

  private:
    int                           m;
    int                           n;
    std::unique_ptr<value_type[]> mem;
};


S compute(const S& lhs, const S& rhs);


