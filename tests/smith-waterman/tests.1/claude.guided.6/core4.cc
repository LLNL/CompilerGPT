
#include "constants.h"

namespace {

inline int matchMissmatchScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, 
                    long long int i, long long int j, long long int m,
                    int* H, int* P, 
                    long long int* maxPos) 
{
    // Pre-compute indices
    const long long int index = m * i + j;
    const long long int diag_index = index - m - 1;
    const long long int up_index = index - m;
    const long long int left_index = index - 1;
    
    // Load values once
    const char char_a = a[j-1];
    const char char_b = b[i-1];
    const int score = matchMissmatchScore(char_a, char_b);
    
    // Compute scores
    const int diag = H[diag_index] + score;
    const int up = H[up_index] + gapScore;
    const int left = H[left_index] + gapScore;

    // Find maximum
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

    // Update maximum position
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char *a, const char *b, 
            int* H, int* P, 
            long long int n, long long int m, long long int& maxPos)
{
    // Cache-friendly tile size
    const long long int TILE_SIZE = 64;
    
    for (long long int ii = 1; ii < n; ii += TILE_SIZE) {
        const long long int iEnd = (ii + TILE_SIZE < n) ? ii + TILE_SIZE : n;
        
        for (long long int jj = 1; jj < m; jj += TILE_SIZE) {
            const long long int jEnd = (jj + TILE_SIZE < m) ? jj + TILE_SIZE : m;
            
            for (long long int i = ii; i < iEnd; ++i) {
                #ifdef __GNUC__
                #pragma GCC ivdep
                #endif
                for (long long int j = jj; j < jEnd; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &maxPos);
                }
            }
        }
    }
}
