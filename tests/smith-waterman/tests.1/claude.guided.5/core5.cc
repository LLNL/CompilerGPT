
#include "constants.h"
#include <algorithm>

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

// Separate computation and update to help vectorization
inline void computeScores(const char* __restrict__ a, const char* __restrict__ b,
                         const long long int i, const long long int j, const long long int m,
                         const int* __restrict__ H,
                         int& score, int& pred) {
    const long long int index = m * i + j;
    const int up = H[index - m] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[index - m - 1] + matchMissmatchScore(a, b, i, j);

    // Initialize with diagonal case
    score = diag;
    pred = DIAGONAL;

    // Branchless max implementation
    const bool up_better = up > score;
    const bool left_better = left > score;
    
    score = up_better ? up : score;
    pred = up_better ? UP : pred;
    
    score = left_better ? left : score;
    pred = left_better ? LEFT : pred;

    // Handle NONE case
    const bool none_case = score <= NONE;
    score = none_case ? NONE : score;
    pred = none_case ? NONE : pred;
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            long long int n, long long int m, long long int& max)
{
    constexpr long long int BLOCK_SIZE = 16;
    int current_max = H[max];
    long long int max_pos = max;

    // Process rows
    for (long long int i = 1; i < n; ++i) {
        // Process blocks within row
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            const long long int j_end = std::min(jj + BLOCK_SIZE, m);
            
            // Separate score computation and array updates to help vectorization
            #pragma clang loop vectorize(enable) interleave(enable)
            for (long long int j = jj; j < j_end; ++j) {
                const long long int index = m * i + j;
                int score, pred;
                
                // Compute scores
                computeScores(a, b, i, j, m, H, score, pred);
                
                // Update arrays
                H[index] = score;
                P[index] = pred;
                
                // Update max if necessary
                const bool is_new_max = score > current_max;
                current_max = is_new_max ? score : current_max;
                max_pos = is_new_max ? index : max_pos;
            }
        }
    }
    
    // Update final max position
    max = max_pos;
}
