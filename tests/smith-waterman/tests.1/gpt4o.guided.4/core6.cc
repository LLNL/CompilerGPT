
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
    // Precomputed indexes
    long long int index = m * i + j;
    long long int index_m = index - m;
    long long int index_m1 = index_m - 1;

    // Extracted common subexpressions
    const char a_prev = a[j - 1];
    const char b_prev = b[i - 1];
    const int diagScore = matchMissmatchScore(a_prev, b_prev);

    // Aggregate score calculation
    const int up = H[index_m] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[index_m1] + diagScore;

    // Finding the maximum score and recording predecessor direction
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

    // Update maximum position if new value is greater
    if (maxVal > H[*maxPos]) {
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
