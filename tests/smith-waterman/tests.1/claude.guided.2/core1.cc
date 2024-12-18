
namespace {

inline int matchMissmatchScore(const char a_char, const char b_char) {
    return (a_char == b_char) ? matchScore : missmatchScore;
}

void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos)
{
    // Pre-calculate index and reuse
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index - m - 1;
    
    // Load values once
    const int current_max_val = H[*maxPos];
    const int score = matchMissmatchScore(a[j-1], b[i-1]);
    
    // Calculate scores
    const int up = H[index_up] + gapScore;
    const int left = H[index_left] + gapScore;
    const int diag = H[index_diag] + score;

    // Branchless max calculation
    int max = NONE;
    int pred = NONE;
    
    // Use conditional moves instead of branches
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

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maxPos if needed
    if (max > current_max_val) {
        *maxPos = index;
    }
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max)
{
    // Cache line alignment
    constexpr size_t BLOCK_SIZE = 64 / sizeof(int);
    
    // Block-based computation
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j < m; j += BLOCK_SIZE) {
            const long long int end = std::min(j + BLOCK_SIZE, m);
            for (long long int k = j; k < end; ++k) {
                similarityScore(a, b, i, k, m, H, P, &max);
            }
        }
    }
}
