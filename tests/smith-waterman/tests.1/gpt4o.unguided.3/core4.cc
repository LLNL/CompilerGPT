
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int j, long long int i_minus_1) {
    return (a[j - 1] == b[i_minus_1]) ? matchScore : missmatchScore;
}

void similarityScoreOptimized(const char* a, const char* b, 
                              long long int i, long long int m,
                              int* H, int* P, long long int* maxPos) 
{
    const long long int prevFirstIndex = m * (i - 1);
    const long long int currentFirstIndex = m * i;
    int* currentH = &H[currentFirstIndex];
    int* currentP = &P[currentFirstIndex];
    const int gapScoreConst = gapScore;
    
    for (long long int j = 1; j < m; ++j) {
        const int diagScore = H[prevFirstIndex + (j - 1)] + matchMissmatchScore(a, b, j, i - 1);
        const int upScore = H[prevFirstIndex + j] + gapScoreConst;
        const int leftScore = currentH[j - 1] + gapScoreConst;

        // Use conditional moves or ternary operations when possible to minimize branching.
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

        currentH[j] = maxValue;
        currentP[j] = predDir;

        // Prefetch next elements to minimize latency.
        __builtin_prefetch(&H[prevFirstIndex + j + 2], 0, 1);
        __builtin_prefetch(&H[currentFirstIndex + j + 2], 0, 1);

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
