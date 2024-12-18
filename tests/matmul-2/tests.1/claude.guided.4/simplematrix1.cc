
SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    // Cache dimensions to avoid repeated calls
    const int m = lhs.rows();
    const int n = rhs.columns();
    const int k = lhs.columns();
    
    SimpleMatrix res{m, n};

    // Loop tiling for better cache utilization
    constexpr int BLOCK_SIZE = 32;
    
    for (int i0 = 0; i0 < m; i0 += BLOCK_SIZE) {
        for (int j0 = 0; j0 < n; j0 += BLOCK_SIZE) {
            for (int k0 = 0; k0 < k; k0 += BLOCK_SIZE) {
                // Block boundaries
                const int i_end = std::min(i0 + BLOCK_SIZE, m);
                const int j_end = std::min(j0 + BLOCK_SIZE, n);
                const int k_end = std::min(k0 + BLOCK_SIZE, k);

                for (int i = i0; i < i_end; ++i) {
                    for (int j = j0; j < j_end; ++j) {
                        double sum = (k0 == 0) ? 0.0 : res(i,j);
                        
                        // Manual unrolling of the inner loop
                        for (int k = k0; k < k_end; k += 4) {
                            if (k + 3 < k_end) {
                                sum += lhs(i, k) * rhs(k, j) +
                                      lhs(i, k+1) * rhs(k+1, j) +
                                      lhs(i, k+2) * rhs(k+2, j) +
                                      lhs(i, k+3) * rhs(k+3, j);
                            } else {
                                for (int kr = k; kr < k_end; ++kr) {
                                    sum += lhs(i, kr) * rhs(kr, j);
                                }
                            }
                        }
                        res(i,j) = sum;
                    }
                }
            }
        }
    }
    return res;
}
