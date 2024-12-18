
#include "constants.h"

namespace {

// Force inline for this small function
__forceinline
int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b,
                    long long int i, long long int j, long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos)
{
    // Pre-calculate index to avoid repeated multiplication
    const long long int index = m * i + j;
    const long long int diag_index = index - m - 1;
    const long long int up_index = index - m;
    const long long int left_index = index - 1;

    // Calculate all scores at once
    const int up = H[up_index] + gapScore;
    const int left = H[left_index] + gapScore;
    const int diag = H[diag_index] + matchMissmatchScore(a, b, i, j);

    // Use branchless max calculation
    int max = NONE;
    int pred = NONE;
    
    // Combine comparisons to reduce branches
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

    // Branchless max update
    *maxPos = (max > H[*maxPos]) ? index : *maxPos;
}

} // anonymous namespace

void compute(const char *a, const char* b, 
            int* __restrict__ H, int* __restrict__ P, 
            long long int n, long long int m, 
            long long int& max)
{
    // Loop unrolling for better instruction pipelining
    long long int i;
    for (i = 1; i < n; ++i) {
        long long int j;
        // Handle alignment and process 4 elements at a time
        for (j = 1; j + 3 < m; j += 4) {
            similarityScore(a, b, i, j, m, H, P, &max);
            similarityScore(a, b, i, j+1, m, H, P, &max);
            similarityScore(a, b, i, j+2, m, H, P, &max);
            similarityScore(a, b, i, j+3, m, H, P, &max);
        }
        // Handle remaining elements
        for (; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
