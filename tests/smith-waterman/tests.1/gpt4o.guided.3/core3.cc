
#include <iostream>

// Assuming these are correctly defined elsewhere, replace them or include your actual constant definitions.
constexpr int matchScore = 1;
constexpr int missmatchScore = -1;
constexpr int gapScore = -1;

constexpr int NONE = 0;
constexpr int DIAGONAL = 1;
constexpr int UP = 2;
constexpr int LEFT = 3;

namespace {

inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    return a[j-1] == b[i-1] ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, 
                     long long int i, long long int j, long long int m,
                     int* H, int* P, 
                     long long int &maxPos) 
{
    long long int index = m * i + j;

    int up = H[index - m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index - m - 1] + matchMissmatchScore(a, b, i, j);

    int max = diag;
    int pred = DIAGONAL;

    if (up > max) {
        max = up;
        pred = UP;
    } else if (left > max) {
        max = left;
        pred = LEFT;
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
