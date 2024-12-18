
#include "constants.h" // Ensure this file has correct constant definitions
#include <algorithm>   // For std::max

namespace
{
    // Inline function to compute match or mismatch score
    inline int matchMissmatchScore(char a, char b) {
        return (a == b) ? matchScore : missmatchScore;
    }

    // Function to calculate the similarity score and update matrices
    void similarityScore(const char *a, const char *b, long long int i, long long int j, long long int m,
                         int *H, int *P, long long int *maxPos) 
    {
        // Linear index based on row-major order
        long long int index = m * i + j;

        // Calculating the scores for cell update
        int diag = H[index - m - 1] + matchMissmatchScore(a[j - 1], b[i - 1]);
        int up = H[index - m] + gapScore;
        int left = H[index - 1] + gapScore;

        // Determine the maximum score and its direction
        int max_val = diag;
        int pred = DIAGONAL;

        if (up > max_val) {
            max_val = up;
            pred = UP;
        }

        if (left > max_val) {
            max_val = left;
            pred = LEFT;
        }

        // Update matrices with calculated max value and predecessor
        H[index] = max_val;
        P[index] = pred;

        // Update the maximum position if the new score is higher
        if (max_val > H[*maxPos]) {
            *maxPos = index;
        }
    }
}

void compute(const char *a, const char *b, int *H, int *P, long long int n, long long int m, long long int &max)
{
    long long int maxPos = 0; // Initialize variable to track the position of the max score

    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }

    max = maxPos; // Set the maximum position after computation
}
