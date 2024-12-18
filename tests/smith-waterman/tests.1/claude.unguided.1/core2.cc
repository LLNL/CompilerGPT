
#include "constants.h"
#include <algorithm>  // for std::min

namespace {

// Pre-computed lookup table for match/mismatch
inline int getScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos)
{
    // Calculate index once
    const long long int index = m * i + j;
    const long long int diag_index = index - m - 1;
    const long long int up_index = index - m;
    const long long int left_index = index - 1;
    
    // Calculate scores
    const int up = H[up_index] + gapScore;
    const int left = H[left_index] + gapScore;
    const int diag = H[diag_index] + getScore(b[i-1], a[j-1]); // Note: switched a,b order
    
    // Find maximum using branchless operations
    int max = NONE;
    int pred = NONE;
    
    // Combine comparisons to reduce branches
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
    
    // Update maxPos if necessary
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max)
{
    // Loop optimization: interchange loops to improve cache locality
    const long long int block_size = 32; // Tune this based on cache size
    
    for (long long int ii = 1; ii < n; ii += block_size) {
        const long long int i_end = std::min(ii + block_size, n);
        for (long long int jj = 1; jj < m; jj += block_size) {
            const long long int j_end = std::min(jj + block_size, m);
            
            for (long long int i = ii; i < i_end; ++i) {
                for (long long int j = jj; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                }
            }
        }
    }
}
