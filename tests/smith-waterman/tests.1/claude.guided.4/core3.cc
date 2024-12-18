
#include "constants.h"

namespace {

inline int matchMissmatchScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* const maxPos,
                    int* const currentMax)  // Pass current max by reference to avoid loads
{
    const long long int index = m * i + j;
    const long long int diag_idx = index - m - 1;
    const long long int up_idx = index - m;
    const long long int left_idx = index - 1;
    
    const int up = H[up_idx] + gapScore;
    const int left = H[left_idx] + gapScore;
    const int diag = H[diag_idx] + matchMissmatchScore(a[j-1], b[i-1]);

    // Simplified max calculation to help vectorization
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

    if (max > *currentMax) {
        *currentMax = max;
        *maxPos = index;
    }
}

} // namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max)
{
    // Cache the current maximum value
    int currentMax = H[max];
    
    // Smaller tile size to reduce register pressure
    const int TILE_SIZE = 32;
    
    // Process main tiles
    for (long long int i = 1; i < n; i++) {
        // Inner loop processes vectors of 4 elements at a time when possible
        long long int j = 1;
        
        // Process main part of row
        for (; j + 4 <= m; j += 4) {
            similarityScore(a, b, i, j, m, H, P, &max, &currentMax);
            similarityScore(a, b, i, j+1, m, H, P, &max, &currentMax);
            similarityScore(a, b, i, j+2, m, H, P, &max, &currentMax);
            similarityScore(a, b, i, j+3, m, H, P, &max, &currentMax);
        }
        
        // Handle remaining elements
        for (; j < m; j++) {
            similarityScore(a, b, i, j, m, H, P, &max, &currentMax);
        }
    }
    
    // Update final max if needed
    H[max] = currentMax;
}
