
#include "constants.h"

// Ensure these constants are defined in constants.h
// Example (ensure these are uncommented if needed or already exist):
// const int matchScore = 1;
// const int missmatchScore = -1;
// const int gapScore = -2;
// const int NONE = 0;
// const int DIAGONAL = 1;
// const int UP = 2;
// const int LEFT = 3;

namespace {

// Ensure function signature and logic are correctly established
inline int matchMissmatchScore(const char* a, const char* b, long long i, long long j) {
    return (a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b, 
                     long long i, long long j, long long m,
                     int* H, int* P, 
                     long long* maxPos)
{
    // Compute indices
    long long index = m * i + j;
    long long index_m = index - m;
    long long index_m1 = index_m - 1;
    long long index_1 = index - 1;

    // Calculate scores
    int up = H[index_m] + gapScore;
    int left = H[index_1] + gapScore;
    int diag = H[index_m1] + matchMissmatchScore(a, b, i, j);

    // Determine max score and direction
    int max = diag;  // Initialize with diag as it's typically most favorable in alignments
    int pred = DIAGONAL;

    if (up > max) {
        max = up;
        pred = UP;
    }

    if (left > max) {
        max = left;
        pred = LEFT;
    }

    // Store results in matrices
    H[index] = max;
    P[index] = pred;

    // Update max position if new max found
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char* a, const char* b, 
             int* H, int* P, 
             long long n, long long m, long long& max)
{
    // Ensure start position of max is safe
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
