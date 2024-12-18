
#include "constants.h"

namespace {

// Force inline for this small, frequently-called function
__attribute__((always_inline))
inline int matchMissmatchScore(const char a_char, const char b_char) {
    return (a_char == b_char) ? matchScore : missmatchScore;
}

void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos)
{
    // Pre-calculate index to avoid repeated computation
    const long long int index = m * i + j;
    const long long int idx_up = index - m;
    const long long int idx_left = index - 1;
    const long long int idx_diag = idx_up - 1;

    // Load values into registers
    const int up = H[idx_up] + gapScore;
    const int left = H[idx_left] + gapScore;
    const int diag = H[idx_diag] + matchMissmatchScore(a[j-1], b[i-1]);

    // Use branchless max calculation
    int max = NONE;
    int pred = NONE;
    
    // Implement max finding using conditional moves (will be optimized by compiler)
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

    // Branchless max update for maxPos
    *maxPos = (max > H[*maxPos]) ? index : *maxPos;
}

}  // namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max)
{
    // Cache blocking optimization
    const int BLOCK_SIZE = 64;  // Adjust based on cache size
    
    for (long long int ii = 1; ii < n; ii += BLOCK_SIZE) {
        const long long int i_end = std::min(ii + BLOCK_SIZE, n);
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            const long long int j_end = std::min(jj + BLOCK_SIZE, m);
            
            // Process block
            for (long long int i = ii; i < i_end; ++i) {
                for (long long int j = jj; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                }
            }
        }
    }
}
