
#include "constants.h"

namespace {

inline int matchMissmatchScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* const maxPos)
{
    const long long int index = m * i + j;
    const long long int diag_idx = index - m - 1;
    const long long int up_idx = index - m;
    const long long int left_idx = index - 1;
    
    const int up = H[up_idx] + gapScore;
    const int left = H[left_idx] + gapScore;
    const int diag = H[diag_idx] + matchMissmatchScore(a[j-1], b[i-1]);

    int max = NONE;
    int pred = NONE;
    
    // Branchless max calculation
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

    H[index] = max;
    P[index] = pred;

    // Cache current maximum
    const int curr_max = H[*maxPos];
    if (max > curr_max) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max)
{
    // Pre-calculate array bounds
    const long long int n_bound = n;
    const long long int m_bound = m;
    
    // Loop tiling for better cache utilization
    const int TILE_SIZE = 64;
    
    for (long long int i_block = 1; i_block < n_bound; i_block += TILE_SIZE) {
        const long long int i_end = (i_block + TILE_SIZE < n_bound) ? 
                                   i_block + TILE_SIZE : n_bound;
        
        for (long long int j_block = 1; j_block < m_bound; j_block += TILE_SIZE) {
            const long long int j_end = (j_block + TILE_SIZE < m_bound) ? 
                                       j_block + TILE_SIZE : m_bound;
            
            for (long long int i = i_block; i < i_end; ++i) {
                #pragma clang loop vectorize(enable)
                for (long long int j = j_block; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                }
            }
        }
    }
}
