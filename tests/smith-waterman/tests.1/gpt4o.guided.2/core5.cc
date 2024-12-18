
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char *a, const char* b, long long i, long long j) {
    // Safe access without checking as 'compute' ensures valid indices during calls
    return (a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, long long i, long long j, long long m,
                     int* H, int* P, long long* maxPos) 
{
    long long index = m * i + j;
    long long index_m = index - m;
    long long index_diag = index_m - 1;

    // Temporarily hold H[*maxPos] for comparison to reduce memory load instructions
    int maxValAtMaxPos = H[*maxPos];

    int up = H[index_m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index_diag] + matchMissmatchScore(a, b, i, j);

    int max = NONE;
    int pred = NONE;

    // Determine the direction for max
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

    H[index] = max;
    P[index] = pred;

    // Only update maxPos if the new max is greater than the previously recorded one
    if (max > maxValAtMaxPos) {
        *maxPos = index;
    }
}

} // unnamed namespace

void compute(const char *a, const char* b, int* H, int* P, long long n, long long m, long long& maxPos)
{
    maxPos = 0;

    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
