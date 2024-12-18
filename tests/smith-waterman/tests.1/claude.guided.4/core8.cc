
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int i, long long int j) {
    const char a_char = a[j-1];
    const char b_char = b[i-1];
    return (a_char == b_char) ? matchScore : missmatchScore;
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
    
    // Load values once
    const int score = matchMissmatchScore(a, b, i, j);
    const int up = H[up_idx] + gapScore;
    const int left = H[left_idx] + gapScore;
    const int diag = H[diag_idx] + score;

    // Compute max and predecessor together to reduce branches
    int max = diag;
    int pred = DIAGONAL;
    
    // Simple linear comparisons for better vectorization
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

    // Update maximum position if needed
    const int current_max = H[*maxPos];
    if (max > current_max) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char* a, const char* b, int* H, int* P, 
            long long int n, long long int m, long long int& max)
{
    // Process rows sequentially for better cache usage
    for (long long int i = 1; i < n; ++i) {
        // Process each row in a linear fashion
        const long long int row_offset = i * m;
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
