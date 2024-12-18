
#include "constants.h"

// Ensure that the constants and macros like matchScore, missmatchScore, gapScore, NONE, DIAGONAL, UP, and LEFT are defined in "constants.h".

inline int matchMissmatchScore(char aj, char bi) {
    return (aj == bi) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b, long long int i, long long int j, long long int m,
                     int* H, int* P, long long int* maxPos) {

    // Calculate the index positions
    long long int index = m * i + j;
    long long int indexPrevRow = index - m;
    long long int indexDiag = indexPrevRow - 1;

    // Access the characters from the strings
    char currentA = a[j - 1];
    char currentB = b[i - 1];

    // Calculate scores based on the direction
    int up = H[indexPrevRow] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[indexDiag] + matchMissmatchScore(currentA, currentB);

    // Determine the maximum score and corresponding direction
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

    // Update the matrices with the calculated values
    H[index] = max;
    P[index] = pred;

    // Update the maximum position if a new high score is found
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

void compute(const char* a, const char* b, int* H, int* P, long long int n, long long int m, long long int& maxPos) {
    for (long long int i = 1; i < n; ++i) {
        char cachedB = b[i - 1]; 

        for (long long int j = 1; j < m; ++j) {
            char cachedA = a[j - 1]; 

            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
