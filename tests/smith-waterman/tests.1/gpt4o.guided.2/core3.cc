
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char *a, const char* b, long long i, long long j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, long long i, long long j, long long m,
                     int* H, int* P, long long* maxPos) 
{
    long long index = m * i + j;
    long long index_m = index - m;

    int currentH = H[index];
    int up = H[index_m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index_m - 1] + matchMissmatchScore(a, b, i, j);

    int max = NONE;
    int pred = NONE;

    if (diag >= max) {
        max = diag;
        pred = DIAGONAL;
    }

    if (up > max) {  // Changed from >= to > to make a decision if equal
        max = up;
        pred = UP;
    } 

    if (left > max) {  // Changed from >= to > to make a decision if equal
        max = left;
        pred = LEFT;
    }

    H[index] = max;
    P[index] = pred;

    if (max > currentH) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char *a, const char* b, int* H, int* P, long long n, long long m, long long& maxPos)
{
    maxPos = 0;

    for (long long i = 1; i < n; ++i) {
        long long base_index = i * m;
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
