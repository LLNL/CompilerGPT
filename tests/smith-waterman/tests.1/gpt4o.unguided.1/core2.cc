
#include "constants.h"
#include <algorithm> // for std::max

namespace
{
  
inline constexpr int matchMissmatchScore(const char *a, const char* b, size_t i, size_t j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, 
                     size_t i, size_t j, size_t m,
                     int* H, int* P, 
                     size_t* maxPos) 
{
    size_t index = m * i + j;
    size_t indexUp = index - m;
    size_t indexDiag = indexUp - 1;
    size_t indexLeft = index - 1;

    int up = H[indexUp] + gapScore;
    int left = H[indexLeft] + gapScore;
    int diag = H[indexDiag] + matchMissmatchScore(a, b, i, j);

    // Calculate maximum value using separate std::max calls
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

}

void compute(const char *a, const char* b, int* H, int* P, size_t n, size_t m, size_t& max)
{
    for (size_t i = 1; i < n; ++i) {
        for (size_t j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
