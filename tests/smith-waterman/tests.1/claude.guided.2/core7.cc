
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    const char char_a = a[j-1];
    const char char_b = b[i-1];
    return (char_a == char_b) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b,
                    const long long int i, const long long int j, const long long int m,
                    int* H, int* P, long long int* maxPos)
{
    // Pre-calculate index and reuse
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index - m - 1;
    
    // Load matrix values
    const int h_up = H[index_up];
    const int h_left = H[index_left];
    const int h_diag = H[index_diag];
    const int current_max = H[*maxPos];
    
    // Calculate scores
    const int score = matchMissmatchScore(a, b, i, j);
    const int up = h_up + gapScore;
    const int left = h_left + gapScore;
    const int diag = h_diag + score;

    // Calculate max and predecessor
    int max = NONE;
    int pred = NONE;
    
    // Simple branching for better predictability
    if (diag > max) {
        max = diag;
        pred = DIAGONAL;
    }
    if (up > max) {
        max = up;
        pred = UP;
    }
    if (left > max) {
        max = left;
        pred = LEFT;
    }

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maxPos if needed
    if (max > current_max) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b,
            int* H, int* P,
            const long long int n, const long long int m, long long int& max)
{    
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
