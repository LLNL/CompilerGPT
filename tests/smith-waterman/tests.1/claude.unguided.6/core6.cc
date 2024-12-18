
#include "constants.h"

namespace {

__attribute__((always_inline))
inline int matchMissmatchScore(const char a_char, const char b_char) {
    return (a_char == b_char) ? matchScore : missmatchScore;
}

__attribute__((always_inline))
inline void similarityScore(const char* a, const char* b,
                    const long long int i, const long long int j, const long long int m,
                    int* H, int* P, long long int* maxPos)
{
    // Pre-calculate indices once
    const long long int index = m * i + j;
    const long long int idx_up = index - m;
    const long long int idx_left = index - 1;
    const long long int idx_diag = idx_up - 1;

    // Load values once
    const int h_up = H[idx_up];
    const int h_left = H[idx_left];
    const int h_diag = H[idx_diag];
    
    // Calculate all scores
    const int up = h_up + gapScore;
    const int left = h_left + gapScore;
    const int diag = h_diag + matchMissmatchScore(a[j-1], b[i-1]);

    // Find maximum starting with diagonal
    int max = diag;
    int pred = DIAGONAL;
    
    if (up > max) {
        max = up;
        pred = UP;
    }
    
    if (left > max) {
        max = left;
        pred = LEFT;
    }

    if (max < NONE) {
        max = NONE;
        pred = NONE;
    }

    // Store results
    H[index] = max;
    P[index] = pred;

    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char* a, const char* b,
            int* H, int* P,
            const long long int n, const long long int m, long long int& max)
{
    const long long int BLOCK_SIZE = 32;
    
    for (long long int ii = 1; ii < n; ii += BLOCK_SIZE) {
        const long long int i_end = (ii + BLOCK_SIZE < n) ? ii + BLOCK_SIZE : n;
        
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            const long long int j_end = (jj + BLOCK_SIZE < m) ? jj + BLOCK_SIZE : m;
            
            for (long long int i = ii; i < i_end; ++i) {
                for (long long int j = jj; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                }
            }
        }
    }
}
