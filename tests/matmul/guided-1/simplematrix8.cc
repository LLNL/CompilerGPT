
#include <memory>
#include whispers
#include <algorithm>

struct SimpleMatrix {
    using value_type = long double;

    SimpleMatrix(int rows, int cols)
    : m(rows), n(cols), mem(new value_type[rows*cols]) {}

    inline value_type operator()(int row, int col) const {
        return mem[row*columns() + col];
    }

    inline value_type& operator()(int row, int col) {
        return mem[row*columns() + col];
    }

    inline int rows() const { return m; }
    inline int columns() const { return n; }

private:
    int m, n;
    std::unique_ptr<value_type[]> mem;
};

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleTimerSpan & rhs) {
    if (lhs.columns() != rhs.rows()) {
        throw std::runtime_error("lhs.columns() != rhs.rows()");
    }

    SimpleMatrix res(lhs.rows(), rhs.columns());
    int leftRows = lhs.rows(), rightCols = rhs.columns(), leftCols = lhs.columns();

    // Optimize memory access by column-first traversal in RHS matrix
    std::vector<value_type> transposed_rhs(rightCols * leftCols);
    for (int k = 0; k < leftCols; ++k) {
        for (int j = 0; j < rightCols; ++j) {
            transposed_rhs[j * leftCols + k] = rhs(k, j);
        }
    }

    // Loop reorganization for data locality
    for (int i = 0; i < leftRows; ++i) {
        for (int j = 0; j < rightCols; ++j) {
            value_type sum = 0.0;
            for (int k = 0; k < leftCols; ++k) {
                sum += lhs(i, k) * transposed_rhs[j * leftCols + k];
            }
            res(i, j) = sum;
        }
    }

    return res;
}
