
#include "constants.h"

namespace {
  
inline int matchMissmatchScore(const char *a, const char* b, long long i, long long j) {
    // Handle boundary cases to avoid out-of-bounds access, ideally should be checked when calling the function.
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, long long i, long long j, long long m,
                     int* H, int* P, long long* maxPos) 
{
    long long index = m * i + j;
    
    int up = H[index - m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index - m - 1] + matchMissmatchScore(a, b, i, j);

    int max = NONE;
    int pred = NONE;

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

    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

} // unnamed namespace

void compute(const char *a, const char* b, int* H, int* P, long long n, long long m, long long& maxPos)
{
    // Set initial max to zero or a minimum possible value in the dynamic programming table context
    maxPos = 0;

    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
