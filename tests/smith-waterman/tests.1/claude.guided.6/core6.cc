
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
    const long long int index = m * i + j;
    const long long int diag_index = index - m - 1;
    const long long int up_index = index - m;
    const long long int left_index = index - 1;

    // Load characters and compute match/mismatch score once
    const int match = matchMissmatchScore(a[j-1], b[i-1]);
    
    // Compute all scores at once
    const int diag = H[diag_index] + match;
    const int up = H[up_index] + gapScore;
    const int left = H[left_index] + gapScore;

    // Initial values
    int max = NONE;
    int pred = NONE;
    
    // Compare in fixed order to help branch prediction
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

    // Update maximum position if needed
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char *a, const char *b, 
            int* H, int* P, 
            long long int n, long long int m, long long int& maxPos)
{
    const long long int TILE_SIZE = 32;  // Reduced tile size for better register usage
    
    for (long long int ii = 1; ii < n; ++ii) {
        for (long long int jj = 1; jj < m; jj += TILE_SIZE) {
            const long long int jEnd = (jj + TILE_SIZE < m) ? jj + TILE_SIZE : m;
            
            // Inner loop with explicit vectorization hint
            #ifdef __GNUC__
            #pragma GCC ivdep
            #endif
            for (long long int j = jj; j < jEnd; ++j) {
                similarityScore(a, b, ii, j, m, H, P, &maxPos);
            }
        }
    }
}
