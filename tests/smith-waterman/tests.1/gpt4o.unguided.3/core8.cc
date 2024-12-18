
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int i, long long int j) {
    return (a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

void similarityScoreOptimized(const char* a, const char* b, 
                              long long int i, long long int m,
                              int* H, int* P, long long int* maxPos) 
{
    const long long int prevRowStart = m * (i - 1);
    const long long int currentRowStart = m * i;
    const int gapScoreConst = gapScore;

    for (long long int j = 1; j < m; ++j) {
        // Calculate matrix indices only once per access
        const long long int diagIndex = prevRowStart + (j - 1);
        const long long int upIndex = prevRowStart + j;
        const long long int leftIndex = currentRowStart + (j - 1);

        const int diagScore = H[diagIndex] + matchMissmatchScore(a, b, i, j);
        const int upScore = H[upIndex] + gapScoreConst;
        const int leftScore = H[leftIndex] + gapScoreConst;

        // Initialize max score and direction predictors
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

        H[currentRowStart + j] = maxValue;
        P[currentRowStart + j] = predDir;

        if (maxValue > H[*maxPos]) {
            *maxPos = currentRowStart + j;
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
