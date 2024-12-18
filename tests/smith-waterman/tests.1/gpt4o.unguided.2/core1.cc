
#include "constants.h"
#include <algorithm>  // For std::max

namespace
{
  
inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

inline void similarityScore(const char *a, const char* b, 
                            long long int i, long long int j, long long int m,
                            int* H, int* P, 
                            long long int* maxPos) 
{
    // Cache frequently used index value
    const long long int index = m * i + j;

    // Precompute previous indices to avoid recalculating them
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index_up - 1;

    const int up = H[index_up] + gapScore;
    const int left = H[index_left] + gapScore;
    const int diag = H[index_diag] + matchMissmatchScore(a, b, i, j);

    // Use std::max to reduce number of if statements
    int max = std::max({diag, up, left, NONE});
    int pred;

    if (max == diag) {
        pred = DIAGONAL;
    } else if (max == up) {
        pred = UP;
    } else { // max == left
        pred = LEFT;
    }

    // Insert the value in the similarity and predecessor matrices
    H[index] = max;
    P[index] = pred;

    // Update maximum score to be used as seed on backtrack
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b, int* H, int* P, long long int n, long long int m, long long int& max)
{
    for (long long int i = 1; i < n; ++i) {
        long long int baseIndex = i * m;  // Calculate baseIndex once per row
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
