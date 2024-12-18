
#include "constants.h"

namespace
{
  
inline int matchMissmatchScore(const char * __restrict a, const char* __restrict b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char * __restrict a, const char* __restrict b, 
                     long long int i, long long int j, long long int m,
                     int* __restrict H, int* __restrict P, 
                     long long int* __restrict maxPos
                    ) 
{
    long long int index = m * i + j;
    int up = H[index - m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index - m - 1] + matchMissmatchScore(a, b, i, j);

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
    
    H[index] = max;
    P[index] = pred;

    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char * __restrict a, const char* __restrict b, int* __restrict H, int* __restrict P, long long int n, long long int m, long long int& max)
{
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j + 1 < m; j += 2) {
            similarityScore(a, b, i, j, m, H, P, &max);
            similarityScore(a, b, i, j + 1, m, H, P, &max);
        }
        if (m % 2 != 0) {
            similarityScore(a, b, i, m - 1, m, H, P, &max);
        }
    }
}
