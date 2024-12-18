
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char c1, const char c2) {
    return (c1 == c2) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b,
                    const long long int i, const long long int j, const long long int m,
                    int* H, int* P, long long int* maxPos)
{
    // Pre-fetch characters to reduce loads and enable LICM
    const char char_a = a[j-1];
    const char char_b = b[i-1];
    
    // Pre-calculate index and reuse
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index - m - 1;
    
    // Load matrix values
    const int h_up = H[index_up];
    const int h_left = H[index_left];
    const int h_diag = H[index_diag];
    const int current_max = H[*maxPos];
    
    // Calculate scores - moved character comparison to separate step
    const int match_score = matchMissmatchScore(char_a, char_b);
    const int up = h_up + gapScore;
    const int left = h_left + gapScore;
    const int diag = h_diag + match_score;

    // Branchless max calculation to reduce register pressure
    const int max_diag_up = (diag > up) ? diag : up;
    const int max_final = (max_diag_up > left) ? max_diag_up : left;
    
    // Determine predecessor based on max value
    int pred;
    if (max_final == diag) {
        pred = DIAGONAL;
    } else if (max_final == up) {
        pred = UP;
    } else {
        pred = LEFT;
    }

    // Store results
    H[index] = max_final;
    P[index] = pred;

    // Update maxPos if needed
    if (max_final > current_max) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b,
            int* H, int* P,
            const long long int n, const long long int m, long long int& max)
{    
    for (long long int i = 1; i < n; ++i) {
        // Prefetch next row to help cache
        __builtin_prefetch(&b[i], 0, 3);
        
        for (long long int j = 1; j < m; ++j) {
            // Prefetch next character
            __builtin_prefetch(&a[j], 0, 3);
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
