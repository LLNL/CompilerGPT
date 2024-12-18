
#include "constants.h"

namespace {

inline int matchMissmatchScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

void compute(const char* a, const char* b, int* H, int* P, 
            long long int n, long long int m, long long int& maxPos) {
    // Pre-compute string offsets to avoid repeated calculations
    const char* const a_shifted = a - 1;
    const char* const b_shifted = b - 1;
    
    // Track maximum score without accessing H[maxPos] repeatedly
    int maxScore = H[maxPos];
    
    // Pre-calculate stride
    const long long int stride = m;
    
    for (long long int i = 1; i < n; ++i) {
        // Cache the previous row position
        long long int prevRow = (i-1) * stride;
        long long int currRow = i * stride;
        
        // Cache previous element
        int prev_elem = H[prevRow];
        char b_char = b_shifted[i];
        
        for (long long int j = 1; j < m; ++j) {
            // Current index
            const long long int curr_idx = currRow + j;
            
            // Calculate scores using cached values
            const int up = H[curr_idx - stride] + gapScore;
            const int left = H[curr_idx - 1] + gapScore;
            const int diag = prev_elem + matchMissmatchScore(a_shifted[j], b_char);
            
            // Update prev_elem for next iteration
            prev_elem = H[prevRow + j];
            
            // Find maximum using branchless operations
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
            
            // Store results
            H[curr_idx] = max;
            P[curr_idx] = pred;
            
            // Update maximum if necessary
            if (max > maxScore) {
                maxScore = max;
                maxPos = curr_idx;
            }
        }
    }
}

}
