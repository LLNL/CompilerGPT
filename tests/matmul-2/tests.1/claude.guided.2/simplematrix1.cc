
SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Block size for cache optimization
    constexpr int BLOCK_SIZE = 32;

    // Zero initialize once
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < N; ++j)
            res(i,j) = 0;

    // Blocked matrix multiplication
    for (int ii = 0; ii < M; ii += BLOCK_SIZE) {
        for (int jj = 0; jj < N; jj += BLOCK_SIZE) {
            for (int kk = 0; kk < K; kk += BLOCK_SIZE) {
                // Block boundaries
                const int iEnd = std::min(ii + BLOCK_SIZE, M);
                const int jEnd = std::min(jj + BLOCK_SIZE, N);
                const int kEnd = std::min(kk + BLOCK_SIZE, K);

                for (int i = ii; i < iEnd; ++i) {
                    for (int k = kk; k < kEnd; ++k) {
                        const double lhs_ik = lhs(i,k);
                        for (int j = jj; j < jEnd; ++j) {
                            res(i,j) += lhs_ik * rhs(k,j);
                        }
                    }
                }
            }
        }
    }
    return res;
}
