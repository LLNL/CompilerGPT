
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long j, long long i) {
    return (a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

void similarityScore(const char* __restrict__ a, const char* __restrict__ b, 
                     long long i, long long j, long long m,
                     int* __restrict__ H, int* __restrict__ P, 
                     long long* maxPos)
{
    // Precompute indices
    const long long index = m * i + j;
    const long long index_m = index - m;
    const long long index_m1 = index_m - 1;
    const long long index_1 = index - 1;

    // Fetch previous values
    const int up = H[index_m] + gapScore;
    const int left = H[index_1] + gapScore;
    const int diag = H[index_m1] + matchMissmatchScore(a, b, j, i);

    // Determine max score and direction
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

    // Update maxPosition
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b, 
             int* __restrict__ H, int* __restrict__ P, 
             long long n, long long m, long long& max)
{
    // Make sure max position is initialized to a safe base
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
