
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int j, long long int i_minus_1) {
    return (a[j - 1] == b[i_minus_1]) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b, 
                     long long int i, long long int j, long long int m,
                     int* H, int* P, long long int* maxPos) 
{
    const long long int index = m * i + j;
    const long long int prevFirstIndex = m * (i - 1);
    
    const int up = H[prevFirstIndex + j] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[prevFirstIndex + (j - 1)] + matchMissmatchScore(a, b, j, i - 1);

    // Set initial max and pred
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

    // Update the similarity and predecessor matrices
    H[index] = maxValue;
    P[index] = predDir;

    // Update maximum score position
    if (maxValue > H[*maxPos]) {
        *maxPos = index;
    }
}

void similarityScoreUnrolled(const char* a, const char* b, 
                             long long int i, long long int m, 
                             int* H, int* P, long long int* maxPos) 
{
    const long long int prevFirstIndex = m * (i - 1);

    for (long long int j = 1; j < m; j += 2) {
        // First iteration (j)
        {
            const long long int index = m * i + j;
            const int up = H[prevFirstIndex + j] + gapScore;
            const int left = H[index - 1] + gapScore;
            const int diag = H[prevFirstIndex + (j - 1)] + matchMissmatchScore(a, b, j, i - 1);

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

            H[index] = maxValue;
            P[index] = predDir;

            if (maxValue > H[*maxPos]) {
                *maxPos = index;
            }
        }

        // Second iteration (j + 1), safe as long as j < m - 1
        if (j + 1 < m) {
            const long long int index = m * i + (j + 1);
            const int up = H[prevFirstIndex + (j + 1)] + gapScore;
            const int left = H[index - 1] + gapScore;
            const int diag = H[prevFirstIndex + j] + matchMissmatchScore(a, b, j + 1, i - 1);

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

            H[index] = maxValue;
            P[index] = predDir;

            if (maxValue > H[*maxPos]) {
                *maxPos = index;
            }
        }
    }
}

}

void compute(const char* a, const char* b, int* H, int* P, long long int n, long long int m, long long int& max)
{
    for (long long int i = 1; i < n; ++i) {
        similarityScoreUnrolled(a, b, i, m, H, P, &max);
    }
}
