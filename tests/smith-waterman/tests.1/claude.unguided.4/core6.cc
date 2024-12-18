
#include "constants.h"

namespace {

// Pre-computed match/mismatch lookup table - aligned for better memory access
alignas(64) static const int score_table[2] = {missmatchScore, matchScore};

__attribute__((always_inline)) 
inline int matchMissmatchScore(const char a, const char b) {
    return score_table[a == b];
}

__attribute__((always_inline))
inline void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos) {
    // Pre-calculate indices
    const long long int index = m * i + j;
    const long long int idx_up = index - m;
    
    // Calculate scores
    const int up = H[idx_up] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[idx_up - 1] + matchMissmatchScore(b[i-1], a[j-1]);

    // Efficient max calculation
    int max = NONE;
    int pred = NONE;
    
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

    // Update maximum position if necessary
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max) {
    // Optimize for cache line size
    constexpr long long int BLOCK_SIZE = 16;
    
    // Main computation loop with blocking
    for (long long int i = 1; i < n; ++i) {
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            const long long int j_end = (jj + BLOCK_SIZE < m) ? jj + BLOCK_SIZE : m;
            
            // Process block
            for (long long int j = jj; j < j_end; ++j) {
                similarityScore(a, b, i, j, m, H, P, &max);
            }
        }
    }
}
