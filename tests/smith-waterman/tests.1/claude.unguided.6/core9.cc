
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, 
                    long long int i, long long int j, long long int m,
                    int* H, int* P, 
                    long long int* maxPos) 
{
    // Pre-calculate all indices
    const long long int index = m * i + j;
    const long long int idx_up = index - m;
    const long long int idx_left = index - 1;
    const long long int idx_diag = idx_up - 1;

    // Calculate all scores
    const int up = H[idx_up] + gapScore;
    const int left = H[idx_left] + gapScore;
    const int diag = H[idx_diag] + matchMissmatchScore(a, b, i, j);

    // Find maximum - initialize with diagonal case
    int max = diag;
    int pred = DIAGONAL;
    
    // Compare with up
    if (up > max) {
        max = up;
        pred = UP;
    }
    
    // Compare with left
    if (left > max) {
        max = left;
        pred = LEFT;
    }

    // Check against NONE
    if (max < NONE) {
        max = NONE;
        pred = NONE;
    }

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maxPos if needed
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b, 
            int* H, int* P, 
            long long int n, long long int m, long long int& max)
{
    const long long int BLOCK_SIZE = 32;
    long long int i_end, j_end;
    
    for (long long int ii = 1; ii < n; ii += BLOCK_SIZE) {
        i_end = (ii + BLOCK_SIZE < n) ? ii + BLOCK_SIZE : n;
        
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            j_end = (jj + BLOCK_SIZE < m) ? jj + BLOCK_SIZE : m;
            
            // Process block
            for (long long int i = ii; i < i_end; ++i) {
                for (long long int j = jj; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                }
            }
        }
    }
}
