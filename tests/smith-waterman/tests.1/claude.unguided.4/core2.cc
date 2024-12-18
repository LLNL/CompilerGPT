
#include "constants.h"

namespace {

// Pre-computed match/mismatch lookup table
static const int score_table[2] = {missmatchScore, matchScore};

inline int matchMissmatchScore(const char a, const char b) {
    return score_table[a == b];
}

void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos) {
    // Pre-calculate indices to avoid repeated computations
    const long long int index = m * i + j;
    const long long int idx_up = index - m;
    const long long int idx_left = index - 1;
    const long long int idx_diag = idx_up - 1;

    // Calculate scores
    const int up = H[idx_up] + gapScore;
    const int left = H[idx_left] + gapScore;
    const int diag = H[idx_diag] + matchMissmatchScore(b[i-1], a[j-1]);

    // Branchless maximum calculation
    int max = NONE;
    int pred = NONE;
    
    // Using conditional moves instead of branches
    bool diag_better = diag > max;
    max = diag_better ? diag : max;
    pred = diag_better ? DIAGONAL : pred;
    
    bool up_better = up > max;
    max = up_better ? up : max;
    pred = up_better ? UP : pred;
    
    bool left_better = left > max;
    max = left_better ? left : max;
    pred = left_better ? LEFT : pred;

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maximum position if necessary
    *maxPos = (max > H[*maxPos]) ? index : *maxPos;
}

} // namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max) {
    // Cache line aligned buffer sizes
    constexpr long long int BLOCK_SIZE = 64 / sizeof(int);
    
    // Block-based iteration for better cache utilization
    for (long long int i = 1; i < n; ++i) {
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            // Calculate end of block, ensuring we don't exceed matrix bounds
            const long long int j_end = (jj + BLOCK_SIZE < m) ? jj + BLOCK_SIZE : m;
            for (long long int j = jj; j < j_end; ++j) {
                similarityScore(a, b, i, j, m, H, P, &max);
            }
        }
    }
}
