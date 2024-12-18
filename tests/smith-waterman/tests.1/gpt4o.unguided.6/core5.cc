
#include <cstddef>  // For std::size_t
#include <vector>
#include "constants.h"

// If constants are actually constexpr, modify the constants.h accordingly

namespace
{

inline int computeDiagScore(const char* a, const char* b, long long int j, long long int i) {
    return (a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b,
                     long long int i, long long int j, long long int m,
                     int* H, int* P,
                     long long int* maxPos)
{
    const long long int index = m * i + j;
    const int* baseH = &H[index];

    const int up = *(baseH - m) + gapScore;
    const int left = *(baseH - 1) + gapScore;
    const int diag = *(baseH - m - 1) + computeDiagScore(a, b, j, i);

    int max = NONE;
    int pred = NONE;

    // Use a more compact if-else structure
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

    // Store results
    baseH[0] = max;
    P[index] = pred;

    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char* a, const char* b, int* H, int* P, long long int n, long long int m, long long int& maxPos)
{
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
