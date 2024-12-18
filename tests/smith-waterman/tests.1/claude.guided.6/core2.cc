
#include "constants.h"  // Contains NONE, UP, LEFT, DIAGONAL, matchScore, missmatchScore, gapScore

namespace {

inline int matchMissmatchScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, 
                    long long int i, long long int j, long long int m,
                    int* H, int* P, 
                    long long int* maxPos) 
{
    const long long int index = m * i + j;
    const long long int diag_index = index - m - 1;
    const long long int up_index = index - m;
    const long long int left_index = index - 1;
    
    const int up = H[up_index] + gapScore;
    const int left = H[left_index] + gapScore;
    const int diag = H[diag_index] + matchMissmatchScore(b[i-1], a[j-1]);

    int max = NONE;
    int pred = NONE;
    
    // Optimized comparison sequence
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

    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char *a, const char* b, int* H, int* P, 
            long long int n, long long int m, long long int& maxPos)
{
    // Cache line size aligned tile size
    const long long int TILE_SIZE = 64;
    
    for (long long int ii = 1; ii < n; ii += TILE_SIZE) {
        const long long int iEnd = (ii + TILE_SIZE < n) ? ii + TILE_SIZE : n;
        for (long long int jj = 1; jj < m; jj += TILE_SIZE) {
            const long long int jEnd = (jj + TILE_SIZE < m) ? jj + TILE_SIZE : m;
            
            for (long long int i = ii; i < iEnd; ++i) {
                #ifdef __GNUC__
                #pragma GCC ivdep
                #endif
                for (long long int j = jj; j < jEnd; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &maxPos);
                }
            }
        }
    }
}
