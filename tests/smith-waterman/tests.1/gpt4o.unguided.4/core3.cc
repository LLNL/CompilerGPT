
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
    int up = H[index - m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index - m - 1] + matchMissmatchScore(a, b, i, j);

    int maxVal = NONE;
    int pred = NONE;

    // Branch prediction hints for likely/unlikely paths (C++20)
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

    H[index] = maxVal;
    P[index] = pred;

    if (maxVal > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char *b, int *H, int *P, long long n, long long m, long long &max) {
    for (long long i = 1; i < n; ++i) {
        long long baseIdx = i * m;
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
