
#include <memory>
#include <stdexcept>

struct SimpleMatrix {
    using value_type = long double;

    SimpleMatrix(int rows, int cols)
    : m(rows), n(cols), mem(new value_type[rows*cols]) {}

    value_type operator()(int row, int col) const {
        return mem[row*columns() + col];
    }

    value_type& operator()(int row, int col) {
        return mem[row*columns() + col];
    }

    int rows() const { return m; }
    int columns() const { return n; }

  private:
    int m;
    int n;
    std::unique_ptr<value_type[]> mem;
};

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows()) {
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};
    }

    int leftRows = lhs.rows();
    int rightCols = rhs.columns();
    int leftCols = lhs.columns();

    SimpleMatrix res{leftRows, rightCols};

    for (int i = 0; i < leftRows; ++i) {
        for (int k = 0; k < leftCols; ++k) {
            long double temp = lhs(i, k);
            for (int j = 0; j < rightCols; ++j) {
                res(i, j) += temp * rhs(k, j);
            }
        }
    }

    return res;
}
