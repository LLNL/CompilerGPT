
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char *a, const char* b, long long i, long long j) {
    return (a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, long long i, long long j, long long m,
                     int* H, int* P, long long* maxPos) 
{
    long long index = m * i + j;
    long long index_m = index - m;
    long long index_diag = index_m - 1;

    int up = H[index_m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index_diag] + matchMissmatchScore(a, b, i, j);

    // Use a direct comparison to reduce unnecessary temporary variables
    if( diag >= up && diag >= left ) {
        H[index] = diag;
        P[index] = DIAGONAL;
    } else if( up >= left ) {
        H[index] = up;
        P[index] = UP;
    } else {
        H[index] = left;
        P[index] = LEFT;
    }

    // Directly compare and assign the new max position only if conditions are met
    if (H[index] > H[*maxPos]) {
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
