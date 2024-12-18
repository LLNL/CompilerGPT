
#include "constants.h"
#include <algorithm>

namespace
{
inline int matchMissmatchScore(char aChar, char bChar) {
    return (aChar == bChar) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b,
                     long long int i, long long int j, long long int m,
                     int* H, int* P,
                     long long int* maxPos)
{
    // Stores index of element
    long long int index = m * i + j;
    long long int index_m = index - m;

    int up = H[index_m] + gapScore;
    int left = H[index - 1] + gapScore;

    // Pre-compute character lookup, reducing indexed array access usage in loop
    char a_prev = a[j - 1];
    char b_prev = b[i - 1];
    int diag = H[index_m - 1] + matchMissmatchScore(a_prev, b_prev);

    // Calculating the maximum
    int maxVal = std::max({NONE, diag, up, left});
    int pred = NONE;
    if (maxVal == diag) pred = DIAGONAL;
    else if (maxVal == up) pred = UP;
    else if (maxVal == left) pred = LEFT;

    H[index] = maxVal;
    P[index] = pred;

    // Updates maximum score to be used as a seed on backtrack
    if (maxVal > H[*maxPos]) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char *a, const char* b, int* H, int* P, long long int n, long long int m, long long int& max)
{
    long long int baseIndex = 0;
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}

