
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b,
                    long long int i, long long int j, long long int m,
                    int* H, int* P, 
                    long long int* maxPos) 
{
    const long long int index = m * i + j;
    const long long int diag_idx = index - m - 1;
    const long long int up_idx = index - m;
    const long long int left_idx = index - 1;
    
    const int up = H[up_idx] + gapScore;
    const int left = H[left_idx] + gapScore;
    const int diag = H[diag_idx] + matchMissmatchScore(a, b, i, j);

    // Simplified max calculation
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

    // Cache current maximum to reduce memory access
    const int curr_max = H[*maxPos];
    if (max > curr_max) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char* a, const char* b, 
            int* H, int* P,
            long long int n, long long int m, long long int& max)
{
    // Main computation loops
    for (long long int i = 1; i < n; i++) {
        // Help vectorization with linear access pattern
        for (long long int j = 1; j < m; j++) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
