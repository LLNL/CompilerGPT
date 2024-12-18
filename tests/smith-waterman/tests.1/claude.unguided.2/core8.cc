
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
    // Pre-calculate indices
    const long long int index = m * i + j;
    const long long int row_offset = index - m;
    
    // Calculate scores
    const int up = H[row_offset] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[row_offset - 1] + matchMissmatchScore(a, b, i, j);

    // Find maximum
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

    // Update maximum position if needed
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b, 
            int* H, int* P, 
            long long int n, long long int m, 
            long long int& max)
{
    // Main computation loop with manual unrolling
    for (long long int i = 1; i < n; ++i) {
        long long int j = 1;
        // Process elements in groups of 2 for better pipelining
        for (; j + 1 < m; j += 2) {
            similarityScore(a, b, i, j, m, H, P, &max);
            similarityScore(a, b, i, j+1, m, H, P, &max);
        }
        // Handle remaining element if any
        for (; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
