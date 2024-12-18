
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int j, long long int i_minus_1) {
    return (a[j - 1] == b[i_minus_1]) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b, 
                     long long int i, long long int j, long long int m,
                     int* H, int* P, 
                     long long int* maxPos) 
{
    long long int index = m * i + j;

    // Utilize pointer arithmetic to avoid recalculating these frequently accessed values
    int* currentH = &H[index];
    int* currentP = &P[index];
    int firstIndex = m * (i - 1);  

    int up = H[firstIndex + j] + gapScore;
    int left = *(currentH - 1) + gapScore;
    int diag = H[firstIndex + (j - 1)] + matchMissmatchScore(a, b, j, i - 1);

    // Determine the maximum score and corresponding direction
    *currentH = 0;  // Initialize with NONE
    *currentP = NONE;

    if (diag > *currentH) {
        *currentH = diag;
        *currentP = DIAGONAL;
    }

    if (up > *currentH) {
        *currentH = up;
        *currentP = UP;
    }
    
    if (left > *currentH) {
        *currentH = left;
        *currentP = LEFT;
    }

    // Update maximum score position
    if (*currentH > H[*maxPos]) {
        *maxPos = index;
    }
}  /* End of similarityScore */

}

void compute(const char* a, const char* b, int* H, int* P, long long int n, long long int m, long long int& max)
{
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
