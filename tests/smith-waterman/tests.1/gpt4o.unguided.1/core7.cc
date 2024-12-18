
#include "constants.h"

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
    const long long index = m * i + j;
    const long long indexUp = index - m;
    const long long indexDiag = indexUp - 1;
    const long long indexLeft = index - 1;

    // Load once and reuse
    const int hIndexUp = H[indexUp];
    const int hIndexLeft = H[indexLeft];
    const int hIndexDiag = H[indexDiag];

    // Using these pre-loaded values
    const int up = hIndexUp + gapScore;
    const int left = hIndexLeft + gapScore;
    const int diag = hIndexDiag + matchMissmatchScore(a, b, i, j);

    // Inline computation flow of max with inline adjustment
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

    H[index] = max;
    P[index] = pred;

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
