
#include <cstddef>  // For std::size_t
#include "constants.h"

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
    int* baseH = &H[index];
    int up = H[index - m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index - m - 1] + computeDiagScore(a, b, j, i);

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

    *baseH = max;
    P[index] = pred;

    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char* a, const char* b, int* H, int* P, long long int n, long long int m, long long int& maxPos)
{
    // We're assuming the first row and column of H are properly initialized.
    for (long long int i = 1; i < n; ++i) {
        long long int base_row = i * m;
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
