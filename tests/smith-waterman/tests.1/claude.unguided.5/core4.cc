
#include "constants.h"

namespace {

// Pre-computed score table
static const int lookupScore[2] = {missmatchScore, matchScore};

void similarityScore(const char* a, const char* b,
                    const long long int i, const long long int j, 
                    const long long int m,
                    int* H, 
                    int* P,
                    long long int* const maxPos)
{
    // Pre-calculate indices
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_diag = index_up - 1;
    
    // Load all values at once to avoid memory stalls
    const int H_up = H[index_up];
    const int H_left = H[index - 1];
    const int H_diag = H[index_diag];
    
    // Simplified match/mismatch calculation using lookup
    const int match = (a[j-1] == b[i-1]);
    const int score = lookupScore[match];
    
    // Calculate scores
    const int up = H_up + gapScore;
    const int left = H_left + gapScore;
    const int diag = H_diag + score;

    // Optimized max calculation
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

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maximum position if necessary
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

} // anonymous namespace

void compute(const char* const a, const char* const b,
            int* const H, int* const P,
            const long long int n, const long long int m, long long int& max)
{
    // Calculate loop bounds once
    const long long int imax = n - 1;
    const long long int jmax = m - 1;
    const long long int jblock = 4; // Process 4 elements at a time
    
    // Main computation loops with aggressive unrolling
    for (long long int i = 1; i <= imax; ++i) {
        // Handle blocks of 4
        long long int j = 1;
        for (; j <= jmax - (jblock-1); j += jblock) {
            similarityScore(a, b, i, j, m, H, P, &max);
            similarityScore(a, b, i, j+1, m, H, P, &max);
            similarityScore(a, b, i, j+2, m, H, P, &max);
            similarityScore(a, b, i, j+3, m, H, P, &max);
        }
        // Handle remaining elements
        for (; j <= jmax; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
