
#include "constants.h"

namespace {

inline int matchMissmatchScore(char aj, char bi) {
    return (aj == bi) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b, long long int i, long long int j, long long int m,
                     int* H, int* P, long long int* maxPos) {

    long long int index = m * i + j;
    long long int indexPrevRow = index - m;
    long long int indexDiag = indexPrevRow - 1;

    // Access b[i-1] once, cache it if safe:
    char currentA = a[j - 1];
    char currentB = b[i - 1];  

    int up = H[indexPrevRow] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[indexDiag] + matchMissmatchScore(currentA, currentB);

    // Use direct calculations to determine maximum and keep a single conditional approach
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

    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}  // End of anonymous namespace

void compute(const char* a, const char* b, int* H, int* P, long long int n, long long int m, long long int& maxPos) {
    for (long long int i = 1; i < n; ++i) {
        char cachedB = b[i - 1];  // Move the invariant load outside the inner loop

        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
