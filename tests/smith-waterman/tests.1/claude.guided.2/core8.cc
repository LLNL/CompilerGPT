
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b,
                    const long long int i, const long long int j, const long long int m,
                    int* H, int* P, const int current_max_val, long long int* maxPos)
{
    // Pre-calculate indices once
    const long long int index = m * i + j;
    const long long int index_m = index - m;
    
    // Load matrix values sequentially to help prefetching
    const int h_up = H[index_m];
    const int h_left = H[index - 1];
    const int h_diag = H[index_m - 1];
    
    // Calculate scores - combine operations to reduce register pressure
    const int up = h_up + gapScore;
    const int left = h_left + gapScore;
    const int diag = h_diag + matchMissmatchScore(a, b, i, j);

    // Initialize max and pred with first comparison
    int max = (diag > up) ? diag : up;
    int pred = (diag > up) ? DIAGONAL : UP;
    
    // Single additional comparison for left value
    if (left > max) {
        max = left;
        pred = LEFT;
    }

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maxPos if needed
    if (max > current_max_val) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b,
            int* H, int* P,
            const long long int n, const long long int m, long long int& max)
{    
    for (long long int i = 1; i < n; ++i) {
        // Load max value once per row
        const int current_max_val = H[max];
        
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, current_max_val, &max);
        }
    }
}
