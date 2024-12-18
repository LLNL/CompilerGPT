
#include "constants.h"
#include <algorithm>

namespace {

inline int matchMissmatchScore(char aChar, char bChar) {
    return (aChar == bChar) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b,
                     long long int i, long long int j, long long int m,
                     int* H, int* P,
                     long long int* maxPos)
{
    // Pre-fetch indices
    long long int index = m * i + j;
    long long int index_m = index - m;
    long long int index_m1 = index_m - 1;

    int up = H[index_m] + gapScore;
    int left = H[index - 1] + gapScore;

    // Cache character access to avoid repetitive loads
    int diag = H[index_m1] + matchMissmatchScore(a[j - 1], b[i - 1]);

    // Calculating the maximum
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

    // Preserve only if current value greater; avoid frequent checks and potential loads
    int currentMax = H[*maxPos];
    if (maxVal > currentMax) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char *a, const char* b, int* H, int* P,
             long long int n, long long int m, long long int& max)
{
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}

