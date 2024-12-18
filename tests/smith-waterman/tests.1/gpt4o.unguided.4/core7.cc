
#include "constants.h"

// Declare the function with extern linkage if it is used across multiple files
extern "C" void compute(
    const char *a, const char *b, int *H, int *P, long long n, long long m, long long &maxPos
);

namespace {

void similarityScore(
    const char *a, const char *b, 
    long long i, long long j, long long m,
    int *H, int *P,
    long long *maxPos
) {
    long long index = m * i + j;

    int up = H[index - m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index - m - 1] + ((a[j - 1] == b[i - 1]) ? matchScore : missmatchScore);

    int maxVal = NONE;
    int pred = NONE;

    if (diag > maxVal) {
        maxVal = diag;
        pred = DIAGONAL;
    }
    if (up > maxVal) {
        maxVal = up;
        pred = UP;
    }
    if (left > maxVal) {
        maxVal = left;
        pred = LEFT;
    }

    H[index] = maxVal;
    P[index] = pred;

    if (maxVal > H[*maxPos]) {
        *maxPos = index;
    }
}

} // End of anonymous namespace

void compute(const char *a, const char *b, int *H, int *P, long long n, long long m, long long &maxPos) {
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
