
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char *a, const char* b, long long i, long long j) {
    // Ensure valid indices before accessing array elements
    return (i > 0 && j > 0 && a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, long long i, long long j, long long m,
                     int* H, int* P, long long* maxPos) 
{
    long long index = m * i + j;
    // Pre-compute indices used to access elements from H to prevent redundant calculations
    long long index_m = index - m;
    long long index_diag = index_m - 1;

    // Ensure current index is valid and within bounds
    int up = H[index_m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index_diag] + matchMissmatchScore(a, b, i, j);

    // Initialize max and pred
    int max = NONE;
    int pred = NONE;

    // Update max and pred based on the highest score
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

    // Update the matrices and max position
    H[index] = max;
    P[index] = pred;

    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

} // unnamed namespace

void compute(const char *a, const char* b, int* H, int* P, long long n, long long m, long long& maxPos)
{
    // Initialize maxPos before computation
    maxPos = 0;

    for (long long i = 1; i < n; ++i) {
        long long base_index = i * m;
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
