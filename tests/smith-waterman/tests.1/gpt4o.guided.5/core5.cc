
#include "constants.h" // Ensure constants are defined before including this header
#include <algorithm>  // For std::max

namespace
{

// Inline function to provide the score based on matching characters
inline int matchMissmatchScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

// Function to compute similarity score and update matrices
void similarityScore(const char *a, const char *b, long long int i, long long int j, long long int m,
                     int *H, int *P, long long int *maxPos) 
{
    // Calculate linear index in the flattened row-major matrix
    long long int index = m * i + j;
    
    // Compute possible scores from three directions
    int diag = H[index - m - 1] + matchMissmatchScore(a[j - 1], b[i - 1]);
    int up = H[index - m] + gapScore;
    int left = H[index - 1] + gapScore;

    // Determine the maximum score among the computed values
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

    // Update the current cell in scoring and predecessor matrices
    H[index] = max_val;
    P[index] = pred;

    // Update the max position if the current max_val is greater
    if (max_val > H[*maxPos]) {
        *maxPos = index;
    }
}

} // End of anonymous namespace

// Function to compute the similarity matrix using input sequences
void compute(const char *a, const char *b, int *H, int *P, long long int n, long long int m, long long int &max)
{
    long long int maxPos = 0; // Initialize the position of the maximal score
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
    max = maxPos; // Return the maximum position found
}
