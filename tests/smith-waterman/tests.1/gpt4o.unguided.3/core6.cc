
#include "constants.h"

namespace {

inline int matchMismatchScore(const char* a, const char* b, long long int j, long long int i_minus_1) {
    return (a[j - 1] == b[i_minus_1]) ? matchScore : mismatchScore;
}

void similarityScoreOptimized(const char* a, const char* b, 
                              long long int i, long long int m,
                              int* H, int* P, long long int* maxPos) 
{
    const long long int prevFirstIndex = m * (i - 1);
    const long long int currentFirstIndex = m * i;
    const int gapScoreConst = gapScore;

    for (long long int j = 1; j < m; ++j) {
        // Pre-calculate indexes to prevent repeated calculations in the loop
        const long long int diagIndex = prevFirstIndex + (j - 1);
        const long long int upIndex = prevFirstIndex + j;
        const long long int leftIndex = currentFirstIndex + (j - 1);

        const int diagScore = H[diagIndex] + matchMismatchScore(a, b, j, i - 1);
        const int upScore = H[upIndex] + gapScoreConst;
        const int leftScore = H[leftIndex] + gapScoreConst;

        int maxValue = diagScore;
        int predDir = DIAGONAL;

        if (upScore > maxValue) {
            maxValue = upScore;
            predDir = UP;
        }
        
        if (leftScore > maxValue) {
            maxValue = leftScore;
            predDir = LEFT;
        }

        H[currentFirstIndex + j] = maxValue;
        P[currentFirstIndex + j] = predDir;

        if (maxValue > H[*maxPos]) {
            *maxPos = currentFirstIndex + j;
        }
    }
}

}

void compute(const char* a, const char* b, int* H, int* P, long long int n, long long int m, long long int& max)
{
    for (long long int i = 1; i < n; ++i) {
        similarityScoreOptimized(a, b, i, m, H, P, &max);
    }
}
