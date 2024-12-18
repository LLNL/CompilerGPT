
#include "constants.h"

namespace {

inline int matchMissmatchScore(char aChar, char bChar) { // Correcting the function name and identifier
    return (aChar == bChar) ? matchScore : missmatchScore; // Using the correct identifier
}

void similarityScore(const char *a, const char* b,
                     long long int i, long long int j, long long int m,
                     int* H, int* P,
                     long long int* maxPos)
{
    long long int index = m * i + j;
    long long int index_m = index - m;
    long long int index_m1 = index_m - 1;

    const char a_prev = a[j - 1];
    const char b_prev = b[i - 1];
    const int diagScore = matchMissmatchScore(a_prev, b_prev); // Corrected function call

    int up = H[index_m];
    int left = H[index - 1];
    int diag = H[index_m1];

    up += gapScore;
    left += gapScore;
    diag += diagScore;

    // Determine maximum score and precedes
    int maxVal = NONE, pred = NONE;

    if (diag > maxVal) {
        maxVal = diag;
        pred = DIAGONAL;
    }
    if (up > maxVal) {
        maxVal = up;
        pred = UP;
    }
    if (left > maxVal) {
        maxVal = left;
        pred = LEFT;
    }

    H[index] = maxVal;
    P[index] = pred;

    // Update maxPos only if genuine improvement is found
    if (maxVal > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b, int* H, int* P,
             long long int n, long long int m, long long int& max)
{
    for (long long int i = 1; i < n; ++i) {
        const char b_prev = b[i - 1];
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
