
#include "constants.h"

namespace {

// Pass characters directly to avoid pointer arithmetic inside matchMissmatchScore
inline int matchMissmatchScore(char aj, char bi) {
    return (aj == bi) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b, long long int i, long long int j, long long int m,
                     int* H, int* P, long long int* maxPos) {

    long long int index = m * i + j;
    long long int indexPrevRow = index - m;
    long long int indexDiag = indexPrevRow - 1;

    char currentA = a[j - 1];
    char currentB = b[i - 1];

    int up = H[indexPrevRow] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[indexDiag] + matchMissmatchScore(currentA, currentB);

    int max = std::max({diag, up, left, NONE});
    int pred = (max == diag) ? DIAGONAL : (max == up) ? UP : LEFT;

    H[index] = max;
    P[index] = pred;

    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}  // End of anonymous namespace

void compute(const char* a, const char* b, int* H, int* P, long long int n, long long int m, long long int& maxPos) {
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
