
#include "constants.h"

namespace {

inline int matchMissmatchScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

// Process multiple elements in one function call to improve vectorization
void similarityScoreBlock(const char *a, const char* b, 
                         long long int i, long long int jStart, long long int jEnd,
                         long long int m, int* H, int* P, 
                         long long int* maxPos) 
{
    // Pre-load b[i-1] since it's constant for this row
    const char char_b = b[i-1];
    
    // Process elements in this row
    for (long long int j = jStart; j < jEnd; ++j) {
        const long long int index = m * i + j;
        const int diag = H[index - m - 1] + matchMissmatchScore(a[j-1], char_b);
        const int up = H[index - m] + gapScore;
        const int left = H[index - 1] + gapScore;

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

        H[index] = max;
        P[index] = pred;

        const int currentMax = H[*maxPos];
        if (max > currentMax) {
            *maxPos = index;
        }
    }
}

} // namespace

void compute(const char *a, const char *b, 
            int* H, int* P, 
            long long int n, long long int m, long long int& maxPos)
{
    // Smaller tile size to reduce register pressure
    const long long int TILE_SIZE = 32;
    
    // Process by tiles
    for (long long int i = 1; i < n; ++i) {
        // Process this row in blocks
        for (long long int jj = 1; jj < m; jj += TILE_SIZE) {
            const long long int jEnd = (jj + TILE_SIZE < m) ? jj + TILE_SIZE : m;
            
            // Process a block of elements
            similarityScoreBlock(a, b, i, jj, jEnd, m, H, P, &maxPos);
        }
    }
}
