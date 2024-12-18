
#include "constants.h"
#include <algorithm> // Make sure to include this for std::max

namespace
{
  
inline constexpr int matchMissmatchScore(const char *a, const char* b, long long i, long long j) noexcept {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, 
                     long long i, long long j, long long m,
                     int* __restrict H, int* __restrict P, 
                     long long* __restrict maxPos) noexcept
{
    long long index = m * i + j;
    long long indexUp = index - m;
    long long indexDiag = indexUp - 1;
    long long indexLeft = index - 1;

    int up = H[indexUp] + gapScore;
    int left = H[indexLeft] + gapScore;
    int diag = H[indexDiag] + matchMissmatchScore(a, b, i, j);

    // Calculate maximum value manually to avoid initializer list issues
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

    // Check and update the maximum position
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b, int* __restrict H, int* __restrict P, long long n, long long m, long long& max) noexcept
{
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
