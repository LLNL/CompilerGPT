
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
    // Pre-computed indices for better clarity and potentially reduced redundant calculations
    size_t index = m * i + j;
    size_t indexUp = index - m;
    size_t indexDiag = indexUp - 1;
    size_t indexLeft = index - 1;

    int up = H[indexUp] + gapScore;
    int left = H[indexLeft] + gapScore;
    int diag = H[indexDiag] + matchMissmatchScore(a, b, i, j);

    // Use std::max to simplify the conditional structure
    int max = std::max({NONE, diag, up, left});
    int pred = (max == diag) ? DIAGONAL : (max == up) ? UP : (max == left) ? LEFT : NONE;

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
