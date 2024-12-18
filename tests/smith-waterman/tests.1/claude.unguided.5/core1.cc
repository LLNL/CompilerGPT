
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char a_char, const char b_char) {
    return (a_char == b_char) ? matchScore : missmatchScore;
}

void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos)
{
    // Pre-calculate index just once
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index - m - 1;
    
    // Load values only once
    const int h_up = H[index_up];
    const int h_left = H[index_left];
    const int h_diag = H[index_diag];
    
    // Calculate scores
    const int up = h_up + gapScore;
    const int left = h_left + gapScore;
    const int diag = h_diag + matchMissmatchScore(a[j-1], b[i-1]);

    // Branchless max calculation
    int max = NONE;
    int pred = NONE;
    
    // Using conditional moves instead of branches
    const bool diag_better = diag > max;
    max = diag_better ? diag : max;
    pred = diag_better ? DIAGONAL : pred;
    
    const bool up_better = up > max;
    max = up_better ? up : max;
    pred = up_better ? UP : pred;
    
    const bool left_better = left > max;
    max = left_better ? left : max;
    pred = left_better ? LEFT : pred;

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maximum position if necessary
    const bool is_new_max = max > H[*maxPos];
    *maxPos = is_new_max ? index : *maxPos;
}

} // anonymous namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max)
{
    // Loop unrolling for better instruction pipelining
    const long long int n_aligned = n - 1;
    const long long int m_aligned = m - 1;
    
    for (long long int i = 1; i < n_aligned; ++i) {
        for (long long int j = 1; j < m_aligned; j += 2) {
            similarityScore(a, b, i, j, m, H, P, &max);
            similarityScore(a, b, i, j + 1, m, H, P, &max);
        }
        // Handle remaining element if m is odd
        if (m_aligned & 1) {
            similarityScore(a, b, i, m_aligned, m, H, P, &max);
        }
    }
}
