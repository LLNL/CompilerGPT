
#include <cstddef>  // For std::size_t
#include "constants.h"

namespace
{

inline int computeDiagScore(const char* a, const char* b, long long int i, long long int j) {
    return (a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b,
                     long long int i, long long int j, long long int m,
                     int* H, int* P,
                     long long int* maxPos)
{
    const long long int index = m * i + j;

    // Calculate the possible scores from different directions
    const int up = H[index - m] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[index - m - 1] + computeDiagScore(a, b, i, j);

    // Determine the maximum score direction
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

    // Store the calculated scores and predecessor direction
    H[index] = max;
    P[index] = pred;

    // Update the position of the maximum score if necessary
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
