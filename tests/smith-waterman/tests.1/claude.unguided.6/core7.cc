
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
    // Pre-calculate index once
    const long long int index = m * i + j;
    const long long int idx_up = index - m;
    const long long int idx_left = index - 1;
    const long long int idx_diag = idx_up - 1;

    // Calculate scores
    const int up = H[idx_up] + gapScore;
    const int left = H[idx_left] + gapScore;
    const int diag = H[idx_diag] + matchMissmatchScore(a, b, i, j);

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

    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b, 
            int* H, int* P, 
            long long int n, long long int m, long long int& max)
{
    // Process matrix in blocks for better cache utilization
    const long long int BLOCK_SIZE = 32;
    
    for (long long int ii = 1; ii < n; ii += BLOCK_SIZE) {
        long long int i_end = ii + BLOCK_SIZE;
        if (i_end > n) i_end = n;
        
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            long long int j_end = jj + BLOCK_SIZE;
            if (j_end > m) j_end = m;
            
            for (long long int i = ii; i < i_end; ++i) {
                for (long long int j = jj; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                }
            }
        }
    }
}
