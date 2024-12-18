
SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    // Cache dimensions to avoid repeated function calls
    const int m = lhs.rows();
    const int n = rhs.columns();
    const int k = lhs.columns();
    
    SimpleMatrix res{m, n};

    // Block size for cache optimization
    constexpr int BLOCK_SIZE = 32;

    // Initialize result to zero first
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            res(i,j) = 0;

    // Blocked matrix multiplication
    for (int ii = 0; ii < m; ii += BLOCK_SIZE) {
        for (int jj = 0; jj < n; jj += BLOCK_SIZE) {
            for (int kk = 0; kk < k; kk += BLOCK_SIZE) {
                // Block boundaries
                const int i_end = std::min(ii + BLOCK_SIZE, m);
                const int j_end = std::min(jj + BLOCK_SIZE, n);
                const int k_end = std::min(kk + BLOCK_SIZE, k);

                for (int i = ii; i < i_end; ++i) {
                    for (int j = jj; j < j_end; ++j) {
                        double temp = res(i,j);
                        for (int k = kk; k < k_end; ++k) {
                            temp += lhs(i,k) * rhs(k,j);
                        }
                        res(i,j) = temp;
                    }
                }
            }
        }
    }
    return res;
}
