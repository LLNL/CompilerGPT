
#include "constants.h"
#include <cstddef> // Include this to use size_t

namespace
{

inline int matchMissmatchScore(const char *a, const char *b, std::size_t i, std::size_t j) {
    return (a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char *b, 
                     std::size_t i, std::size_t j, std::size_t m,
                     int *H, int *P, std::size_t *maxPos) 
{
    std::size_t index = m * i + j;
    std::size_t indexUp = index - m;
    std::size_t indexLeft = index - 1;
    std::size_t indexDiag = index - m - 1;

    const int up = H[indexUp] + gapScore;
    const int left = H[indexLeft] + gapScore;
    const int diag = H[indexDiag] + matchMissmatchScore(a, b, i, j);

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

void compute(const char *a, const char *b, int *H, int *P, std::size_t n, std::size_t m, std::size_t &maxPos) 
{
    for (std::size_t i = 1; i < n; ++i) {
        for (std::size_t j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
