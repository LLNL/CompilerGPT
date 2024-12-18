
#include "constants.h"

namespace
{
  
inline
int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    if (a[j-1] == b[i-1])
        return matchScore;
    else
        return missmatchScore;
}

void similarityScore( const char *a, const char* b, 
                      long long int i, long long int j, long long int m,
                      int* H, int* P, 
                      long long int* maxPos
                    ) 
{
    int up, left, diag;

    //Stores index of element
    long long int index = m * i + j;

    up = H[index - m] + gapScore;

    left = H[index - 1] + gapScore;

    //Get element on the diagonal
    diag = H[index - m - 1] + matchMissmatchScore(a, b, i, j);

    //Calculates the maximum
    int max = NONE;
    int pred = NONE;
    
    if (diag > max) { //same letter ↖
        max = diag;
        pred = DIAGONAL;
    }

    if (up > max) { //remove letter ↑ 
        max = up;
        pred = UP;
    }
    
    if (left > max) { //insert letter ←
        max = left;
        pred = LEFT;
    }
    //Inserts the value in the similarity and predecessor matrixes
    H[index] = max;
    P[index] = pred;

    //Updates maximum score to be used as seed on backtrack 
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}  /* End of similarityScore */

}

void compute(const char *a, const char* b, int* H, int* P, long long int n, long long int m, long long int& max)
{
  for (long long int i = 1; i < n; ++i)
    for (long long int j = 1; j < m; ++j)
      similarityScore(a, b, i, j, m, H, P, &max);
}
