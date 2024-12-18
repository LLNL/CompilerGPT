
#include "constants.h"

namespace {

inline int matchMissmatchScore(char a_char, char b_char) {
    // Direct comparison and returning precomputed constant eliminates calculation overhead
    return (a_char == b_char) ? matchScore : missmatchScore;
}

inline void similarityScore(const char *a, const char* b, 
                            long long i, long long j, long long m,
                            int* H, int* P, long long* maxPos) 
{
    const long long index = m * i + j;
    const long long index_up = index - m;
    const long long index_diag = index_up - 1;

    // Fetch once, avoiding multiple array access overhead
    const int current_char_match_score = matchMissmatchScore(a[j - 1], b[i - 1]);
    const int up = H[index_up] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[index_diag] + current_char_match_score;

    // Determine the maximum score and track the predecessor position
    int max = diag > up ? (diag > left ? diag : left) : (up > left ? up : left);
    int pred = (max == diag) ? DIAGONAL : ((max == up) ? UP : LEFT);

    // Store results in H and P matrices
    H[index] = max;
    P[index] = pred;

    // Update the maxPos if a new maximum score is found
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b, int* H, int* P, long long n, long long m, long long& max) {
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
