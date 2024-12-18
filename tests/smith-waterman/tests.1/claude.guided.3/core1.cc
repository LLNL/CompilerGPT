
namespace {

inline int matchMissmatchScore(char a_char, char b_char) {
    return (a_char == b_char) ? matchScore : missmatchScore;
}

void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos) {
    // Pre-calculate index to avoid repeated computation
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index - m - 1;
    
    // Prefetch next iterations
    __builtin_prefetch(&H[index + 1]);
    __builtin_prefetch(&P[index + 1]);
    
    // Calculate scores
    const int up = H[index_up] + gapScore;
    const int left = H[index_left] + gapScore;
    const int diag = H[index_diag] + matchMissmatchScore(a[j-1], b[i-1]);

    // Find maximum using branchless operations
    int max = NONE;
    int pred = NONE;
    
    // Combine comparisons to reduce branches
    if (diag > max) {
        max = diag;
        pred = DIAGONAL;
    }
    if (up > max) {
        max = up;
        pred = UP;
    }
    if (left > max) {
        max = left;
        pred = LEFT;
    }

    H[index] = max;
    P[index] = pred;

    // Use local variable to reduce pointer dereferencing
    const long long int curr_max_pos = *maxPos;
    if (max > H[curr_max_pos]) {
        *maxPos = index;
    }
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max) {
    // Pre-calculate loop bounds
    const long long int n_bound = n;
    const long long int m_bound = m;
    
    // Use fixed-size blocks for better cache utilization
    constexpr long long int BLOCK_SIZE = 64;
    
    for (long long int i = 1; i < n_bound; i++) {
        for (long long int j = 1; j < m_bound; j += BLOCK_SIZE) {
            const long long int end = std::min(j + BLOCK_SIZE, m_bound);
            for (long long int k = j; k < end; k++) {
                similarityScore(a, b, i, k, m, H, P, &max);
            }
        }
    }
}
