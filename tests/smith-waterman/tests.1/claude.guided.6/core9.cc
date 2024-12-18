
#include "constants.h"

namespace {

inline int matchMissmatchScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

void processRow(const char* a, const char b_char,
               const long long int i, const long long int m,
               const long long int start, const long long int end,
               int* __restrict__ H, int* __restrict__ P,
               int& localMaxScore, long long int& localMaxPos)
{
    #ifdef __GNUC__
    #pragma GCC ivdep
    #endif
    for (long long int j = start; j < end; ++j) {
        const long long int index = m * i + j;
        const long long int diag_index = index - m - 1;
        const long long int up_index = index - m;
        const long long int left_index = index - 1;

        // Pre-load all values
        const int h_diag = H[diag_index];
        const int h_up = H[up_index];
        const int h_left = H[left_index];
        
        // Score computation
        const int diag = h_diag + matchMissmatchScore(a[j-1], b_char);
        const int up = h_up + gapScore;
        const int left = h_left + gapScore;

        // Find maximum using sequential operations
        int max = diag;
        int pred = DIAGONAL;
        
        const bool up_better = up > max;
        max = up_better ? up : max;
        pred = up_better ? UP : pred;
        
        const bool left_better = left > max;
        max = left_better ? left : max;
        pred = left_better ? LEFT : pred;

        // Store results
        H[index] = max;
        P[index] = pred;

        // Update local maximum
        if (max > localMaxScore) {
            localMaxScore = max;
            localMaxPos = index;
        }
    }
}

} // namespace

void compute(const char *a, const char *b, 
            int* __restrict__ H, int* __restrict__ P, 
            const long long int n, const long long int m, 
            long long int& maxPos)
{
    // Cache line size-based blocking
    constexpr long long int BLOCK_SIZE = 32;
    int globalMaxScore = H[maxPos];
    long long int globalMaxPos = maxPos;

    // Process each row
    for (long long int i = 1; i < n; ++i) {
        // Load row-constant character once
        const char b_char = b[i-1];
        int rowMaxScore = globalMaxScore;
        long long int rowMaxPos = globalMaxPos;

        // Process row in blocks
        for (long long int j = 1; j < m; j += BLOCK_SIZE) {
            const long long int end = (j + BLOCK_SIZE < m) ? j + BLOCK_SIZE : m;
            
            // Process block with local maximums
            processRow(a, b_char, i, m, j, end, H, P, rowMaxScore, rowMaxPos);
        }

        // Update global maximum only once per row
        if (rowMaxScore > globalMaxScore) {
            globalMaxScore = rowMaxScore;
            globalMaxPos = rowMaxPos;
        }
    }

    // Final update of maxPos
    maxPos = globalMaxPos;
}
