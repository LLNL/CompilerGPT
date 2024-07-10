
#include <memory>
#include <stdexcept>
#include <vector>
#include <omp.h>

struct SimpleMatrix {
    using value_type = long double;
    std::vector<value_type> mem;
    int m, n;

    SimpleMatrix(int rows, int cols)
        : m(rows), n(cols), mem(rows * cols, 0) {}

    value_type operator()(int row, int col) const {
        return mem[row * n + col];
    }

    value_type& operator()(int row, int col) {
        return mem[row * n + col];
    }

    int rows() const { return m; }
    int columns() const { return n; }
};

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_tree("lhs.columns() != rhs.rows");
    
    SimpleMatrix res(lhs.rows(), rhs.columns());

    #pragma omp parallel for
    for (int i = 0; i < lhs.rows(); ++i) {
        for (int k = 0; k < lhs.columns(); ++k) {
            auto temp = lhs(i, k);
            for (int j = 0; j < rhs.columns(); ++j) {
                res(i, j) += temp * rhs(k, j);
            }
        }
    }

    return res;
}
