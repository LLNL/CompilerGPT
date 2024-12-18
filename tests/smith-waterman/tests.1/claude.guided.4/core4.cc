
#include "constants.h"

namespace {

inline int matchMissmatchScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* const maxPos,
                    int* const currentMax)
{
    const long long int index = m * i + j;
    
    const int up = H[index - m] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[index - m - 1] + matchMissmatchScore(a[j-1], b[i-1]);

    // Simplified max calculation
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

    if (max > *currentMax) {
        *currentMax = max;
        *maxPos = index;
    }
}

} // namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max)
{
    // Cache the current maximum value
    int currentMax = H[max];
    
    // Process elements in continuous chunks for better vectorization
    for (long long int i = 1; i < n; i++) {
        for (long long int j = 1; j < m; j++) {
            similarityScore(a, b, i, j, m, H, P, &max, &currentMax);
        }
    }
}
