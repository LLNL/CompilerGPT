
#include "constants.h"
#include <algorithm>

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void processBlock(const char* __restrict__ a, const char* __restrict__ b,
                 const long long int i, const long long int j_start, const long long int j_end,
                 const long long int m,
                 int* __restrict__ H, int* __restrict__ P,
                 int* __restrict__ maxH, long long int* __restrict__ maxPos)
{
    // Pre-compute row offsets
    const long long int row_offset = m * i;
    const long long int prev_row_offset = m * (i - 1);
    
    // Cache current maximum
    int local_max = *maxH;
    long long int local_max_pos = *maxPos;

    // Process block
    #pragma clang loop vectorize(enable) interleave(enable)
    for (long long int j = j_start; j < j_end; ++j) {
        // Calculate indices
        const long long int curr_idx = row_offset + j;
        const long long int up_idx = curr_idx - m;
        const long long int left_idx = curr_idx - 1;
        const long long int diag_idx = prev_row_offset + (j - 1);

        // Load values
        const int score_up = H[up_idx] + gapScore;
        const int score_left = H[left_idx] + gapScore;
        const int score_diag = H[diag_idx] + matchMissmatchScore(a, b, i, j);

        // Compute scores - minimize branches
        int score = score_diag;
        int pred = DIAGONAL;

        const bool up_better = score_up > score;
        score = up_better ? score_up : score;
        pred = up_better ? UP : pred;

        const bool left_better = score_left > score;
        score = left_better ? score_left : score;
        pred = left_better ? LEFT : pred;

        // Handle NONE case
        const bool is_none = score <= NONE;
        score = is_none ? NONE : score;
        pred = is_none ? NONE : pred;

        // Store results
        H[curr_idx] = score;
        P[curr_idx] = pred;

        // Update maximum using branchless operation
        const bool is_new_max = score > local_max;
        local_max = is_new_max ? score : local_max;
        local_max_pos = is_new_max ? curr_idx : local_max_pos;
    }

    // Update global maximum if necessary
    if (local_max > *maxH) {
        *maxH = local_max;
        *maxPos = local_max_pos;
    }
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            long long int n, long long int m, long long int& max)
{
    // Cache current maximum H value
    int maxH = H[max];
    
    constexpr long long int BLOCK_SIZE = 32;

    // Process matrix in blocks
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j < m; j += BLOCK_SIZE) {
            const long long int j_end = std::min(j + BLOCK_SIZE, m);
            processBlock(a, b, i, j, j_end, m, H, P, &maxH, &max);
        }
    }
}
