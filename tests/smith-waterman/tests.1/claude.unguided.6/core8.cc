
#include "constants.h"

namespace {

// Store scoring constants in registers
static const int GAP = gapScore;
static const int MATCH = matchScore;
static const int MISMATCH = missmatchScore;

inline int matchMissmatchScore(const char *a, const char* b, const long long int i, const long long int j) {
    return (a[j-1] == b[i-1]) ? MATCH : MISMATCH;
}

void similarityScore(const char *a, const char* b, 
                    const long long int i, const long long int j, const long long int m,
                    int* const H, int* const P, 
                    long long int* const maxPos) 
{
    // Pre-calculate all indices at once
    const long long int index = m * i + j;
    const long long int idx_up = index - m;
    const long long int idx_left = index - 1;
    const long long int idx_diag = idx_up - 1;

    // Load all values at once to help prefetcher
    const int score_up = H[idx_up];
    const int score_left = H[idx_left];
    const int score_diag = H[idx_diag];
    const int current_max = H[*maxPos];

    // Calculate all scores
    const int up = score_up + GAP;
    const int left = score_left + GAP;
    const int diag = score_diag + matchMissmatchScore(a, b, i, j);

    // Initialize with most likely case
    int max = diag;
    int pred = DIAGONAL;
    
    // Sequential comparison often better than branching
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

    // Update maxPos if needed
    if (max > current_max) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b, 
            int* const H, int* const P, 
            const long long int n, const long long int m, long long int& max)
{
    // Optimize block size for cache line size (typically 64 bytes)
    const long long int BLOCK_SIZE = 32;
    
    // Main computation loop with blocking
    for (long long int ii = 1; ii < n; ii += BLOCK_SIZE) {
        const long long int i_end = (ii + BLOCK_SIZE < n) ? ii + BLOCK_SIZE : n;
        
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            const long long int j_end = (jj + BLOCK_SIZE < m) ? jj + BLOCK_SIZE : m;
            
            // Process current block
            for (long long int i = ii; i < i_end; ++i) {
                // Manual loop unrolling for better pipelining
                long long int j = jj;
                
                // Process 4 elements at a time when possible
                for (; j + 3 < j_end; j += 4) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                    similarityScore(a, b, i, j+1, m, H, P, &max);
                    similarityScore(a, b, i, j+2, m, H, P, &max);
                    similarityScore(a, b, i, j+3, m, H, P, &max);
                }
                
                // Handle remaining elements
                for (; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                }
            }
        }
    }
}
