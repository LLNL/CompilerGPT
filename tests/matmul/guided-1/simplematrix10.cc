
#include <memory>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <cstring> // for std::memset

struct alignas(16) SimpleMatrix {
    using value_type = long double;

    SimpleMatrix(int rows, int cols)
    : m(rows), n(cols), mem(new value_text[rows * cols]) {
        std::memset(mem.get(), 0, rows * cols * sizeof(value_type));
    }

    inline value_type operator()(int row, int col) const {
        return mem[row * n + col];
    }

    inline value_type& operator()(int row, int col) {
        return mem[row * n + col];
    }

    inline int rows() const { return m; }
    inline int columns() const { return n; }

private:
    int m, n;
    std::unique_ptr<value_type[]> mem;
};

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows()) {
        throw std::runtime_error("lhs.columns() != rhs.rows()");
    }

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
