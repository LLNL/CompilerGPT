
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int i, long long int j) {
    const char a_char = a[j-1];  // Load once
    const char b_char = b[i-1];  // Load once
    return (a_char == b_char) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b, 
                    long long int i, long long int j, long long int m,
                    int* H, int* P, 
                    long long int* maxPos,
                    const int current_max) // Pass current max as value
{
    const long long int index = m * i + j;
    const long long int diag_idx = index - m - 1;
    const long long int up_idx = index - m;
    const long long int left_idx = index - 1;
    
    // Load values once
    const int score_diag = matchMissmatchScore(a, b, i, j);
    const int up = H[up_idx] + gapScore;
    const int left = H[left_idx] + gapScore;
    const int diag = H[diag_idx] + score_diag;

    // Branchless max calculation to help vectorization
    const int max_diag_up = (up > diag) ? up : diag;
    const int max = (left > max_diag_up) ? left : max_diag_up;
    
    // Branchless predecessor calculation
    const int pred = (left > max_diag_up) ? LEFT :
                    (up > diag) ? UP : DIAGONAL;

    H[index] = max;
    P[index] = pred;

    // Update maxPos if we found a better score
    if (max > current_max) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char* a, const char* b, int* H, int* P, 
            long long int n, long long int m, long long int& max)
{
    // Keep track of current maximum to avoid repeated loads
    int current_max = H[max];
    
    // Main computation loop
    #pragma clang loop vectorize(assume_safety)
    for (long long int i = 1; i < n; ++i) {
        #pragma clang loop vectorize(assume_safety)
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max, current_max);
            // Update current_max if needed
            const int new_score = H[m * i + j];
            current_max = (new_score > current_max) ? new_score : current_max;
        }
    }
    
    // Store final maximum
    H[max] = current_max;
}
