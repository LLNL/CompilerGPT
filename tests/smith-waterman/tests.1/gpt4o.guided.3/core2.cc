
#include "constants.h" // Ensure this header defines all necessary constants

namespace {

inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    // Assuming matchScore and missmatchScore are defined in constants.h
    return a[j-1] == b[i-1] ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, 
                     long long int i, long long int j, long long int m,
                     int* H, int* P, 
                     long long int &maxPos) 
{
    long long int index = m * i + j;

    // Assuming gapScore is defined in constants.h
    int up = H[index - m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index - m - 1] + matchMissmatchScore(a, b, i, j);

    int max = diag;
    int pred = DIAGONAL; // Assuming DIAGONAL is defined in constants.h

    if (up > max) {
        max = up;
        pred = UP; // Assuming UP is defined in constants.h
    } else if (left > max) {
        max = left;
        pred = LEFT; // Assuming LEFT is defined in constants.h
    }

    H[index] = max;
    P[index] = pred;

    if (max > H[maxPos]) {
        maxPos = index;
    }
}

} // unnamed namespace

void compute(const char *a, const char* b, int* H, int* P, long long int n, long long int m, long long int &maxPos)
{
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, maxPos);
        }
    }
}
