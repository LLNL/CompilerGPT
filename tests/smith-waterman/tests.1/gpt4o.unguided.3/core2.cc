
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int j, long long int i_minus_1) {
    return (a[j - 1] == b[i_minus_1]) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b, 
                     long long int i, long long int j, long long int m,
                     int* H, int* P, long long int* maxPos) 
{
    long long int index = m * i + j;
    int* currentH = &H[index];
    int* currentP = &P[index];

    const int firstIndex = m * (i - 1);  
    const int upIndex = firstIndex + j;
    const int leftIndex = index - 1;
    const int diagIndex = firstIndex + (j - 1);

    const int up = H[upIndex] + gapScore;
    const int left = H[leftIndex] + gapScore;
    const int diag = H[diagIndex] + matchMissmatchScore(a, b, j, i - 1);

    // Determine the maximum score and corresponding direction
    int maxValue = 0;  // NONE
    int predDir = NONE;

    if (diag > maxValue) {
        maxValue = diag;
        predDir = DIAGONAL;
    }

    if (up > maxValue) {
        maxValue = up;
        predDir = UP;
    }
    
    if (left > maxValue) {
        maxValue = left;
        predDir = LEFT;
    }

    *currentH = maxValue;
    *currentP = predDir;

    // Update maximum score position
    if (maxValue > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char* a, const char* b, int* H, int* P, long long int n, long long int m, long long int& max)
{
    for (long long int i = 1; i < n; ++i) {
        const long long int firstIndex = m * i;
        const long long int prevFirstIndex = m * (i - 1);

        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
