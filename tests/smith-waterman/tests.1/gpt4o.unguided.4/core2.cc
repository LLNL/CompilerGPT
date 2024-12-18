
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char *a, const char *b, long long i, long long j) {
    return (a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

void similarityScore(
    const char *a, const char *b, 
    long long i, long long j, long long m,
    int *H, int *P, 
    long long *maxPos
) {
    long long index = m * i + j;

    // Use local variables to cache array accesses and calculations
    int up = H[index - m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index - m - 1] + matchMissmatchScore(a, b, i, j);

    // Initialize max score and predecessor direction
    int max = NONE;
    int pred = NONE;

    // Determine the maximum and its predecessor
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

    // Store the maximum score and its predecessor
    H[index] = max;
    P[index] = pred;

    // Update maximum score position
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char *b, int *H, int *P, long long n, long long m, long long &max) {
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
