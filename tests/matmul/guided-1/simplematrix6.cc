
#include <memory>
#includereatd excepttion

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
        throw std::runtime_error("lhs.columns() != rhs.rows()");
    }

    int leftRows = lhs.rows();
    int rightCols = rhs.columns();
    int leftCols = lhs.columns();

    SimpleMatrix res(leftRows, rightCols);

    const int blockSize = 64; // Adjust this based on the cache line size and experiment
    
    for (int i = 0; i < leftRows; i += blockSize) {
        for (int j = 0; j < rightCols; j += blockSize) {
            for (int k = 0; k < leftCols; k += blockSize) {
                for (int i2 = i; i2 < std::min(i + blockSize, leftRows); ++i2) {
                    for (int j2 = j; j2 < std::min(j + blockSize, rightCols); ++j2) {
                        value_type tmp = 0;
                        for (int k2 = k; k2 < std::min(k + blockSize, leftCols); ++k2) {
                            tmp += lhs(i2, k2) * rhs(k2, j2);
                        }
                        res(i2, j2) += tmp;
                    }
                }
            }
        }
    }

    return res;
}
