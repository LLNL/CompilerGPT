
#include "constants.h"

namespace {

// Function to calculate the match or mismatch score
inline int matchMissmatchScore(const char *a, const char *b, long long i, long long j) {
    return (a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

// Optimized similarityScore function
void similarityScore(
    const char *a, const char *b, 
    long long i, long long j, long long m,
    int *H, int *P, 
    long long *maxPos
) {
    // Calculate index once and use it for array accesses
    long long index = m * i + j;
    int *currentH = &H[index];
    int *currentP = &P[index];

    // Cache neighboring scores
    int up = H[index - m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index - m - 1] + matchMissmatchScore(a, b, i, j);

    // Use temporary variables to determine the maximum score and direction
    int maxVal = NONE;
    int pred = NONE;

    if (diag > maxVal) [[likely]] {
        maxVal = diag;
        pred = DIAGONAL;
    }
    if (up > maxVal) [[unlikely]] {
        maxVal = up;
        pred = UP;
    }
    if (left > maxVal) [[unlikely]] {
        maxVal = left;
        pred = LEFT;
    }

    // Update the score and predecessor matrices
    *currentH = maxVal;
    *currentP = pred;

    // Update the max position if necessary
    if (maxVal > H[*maxPos]) {
        *maxPos = index;
    }
}

}

// Optimized compute function
void compute(const char *a, const char *b, int *H, int *P, long long n, long long m, long long &max) {
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
