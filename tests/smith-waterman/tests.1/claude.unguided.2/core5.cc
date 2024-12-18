
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b,
                    const long long int i, const long long int j, 
                    const long long int m,
                    int* H, int* P,
                    long long int* maxPos)
{
    const long long int index = m * i + j;
    const long long int row_offset = index - m;
    
    const int up = H[row_offset] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[row_offset - 1] + matchMissmatchScore(a, b, i, j);

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

} // anonymous namespace

void compute(const char *a, const char* b,
            int* H, int* P,
            long long int n, long long int m,
            long long int& max)
{
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j + 3 < m; j += 4) {
            similarityScore(a, b, i, j, m, H, P, &max);
            similarityScore(a, b, i, j+1, m, H, P, &max);
            similarityScore(a, b, i, j+2, m, H, P, &max);
            similarityScore(a, b, i, j+3, m, H, P, &max);
        }
        
        // Handle remaining elements
        for (long long int j = m - (m % 4); j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
