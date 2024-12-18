/***********************************************************************
 * Smith–Waterman algorithm
 * Purpose:     Local alignment of nucleotide or protein sequences
 * Authors:     Daniel Holanda, Hanoch Griner, Taynara Pinheiro
 ***********************************************************************/

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <chrono>

#include "constants.h"

void compute(const char *a, const char* b, int* H, int* P, long long int n, long long int m, long long int& max);

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

void compute0(const char *a, const char* b, int* H, int* P, long long int n, long long int m, long long int& max)
{
  for (long long int i = 1; i < n; ++i)
    for (long long int j = 1; j < m; ++j)
      similarityScore(a, b, i, j, m, H, P, &max);
}

void generate(char* a, char* b, long long int m, long long int n)
{
    //Generates the values of a
    for(long long int i=0;i<m; ++i)
    {
        int aux=rand()%4;
        if(aux==0)
            a[i]='A';
        else if(aux==2)
            a[i]='C';
        else if(aux==3)
            a[i]='G';
        else
            a[i]='T';
    }

    //Generates the values of b
    for(long long int i=0;i<n; ++i) 
    {
        int aux=rand()%4;
        if(aux==0)
            b[i]='A';
        else if(aux==2)
            b[i]='C';
        else if(aux==3)
            b[i]='G';
        else
            b[i]='T';
      }
} /* End of generate */

template <class TPtr>
void assert_equal_sequence(TPtr one, TPtr two, std::size_t len, const char* msg)
{
  if (!std::equal(one, one+len, two))
    throw std::runtime_error(msg);
}

template <class T>
void assert_equal(T lhs, T rhs, const char* msg)
{
  if (lhs != rhs)
    throw std::runtime_error(msg);
}



/*--------------------------------------------------------------------
 * Function:    main
 */
int main(int argc, char* argv[]) 
{
  using time_point = std::chrono::time_point<std::chrono::system_clock>;
  
  long long int m = 19553;
  long long int n = 29959;
    
  char* a = new char[m];
  char* b = new char[n];
    
  //Because now we have zeros
  m++;
  n++;
    
  //Allocates similarity matrix H
  int* H  = new int[m * n]();
  int* P  = new int[m * n]();

  int* H0 = new int[m * n]();
  int* P0 = new int[m * n]();

  //Gen rand arrays a and b
  generate(a, b, m, n);

  //Start position for backtrack
  long long int maxPos  = 0;
  long long int maxPos0 = 0;
    
  time_point   starttime = std::chrono::system_clock::now();  
  compute(a, b, H,  P,  n, m, maxPos);
  time_point   endtime = std::chrono::system_clock::now();
  int          elapsedtime = std::chrono::duration_cast<std::chrono::milliseconds>(endtime-starttime).count();  
  
  compute0(a, b, H0, P0, n, m, maxPos0);
  
  try
  {
    assert_equal(maxPos, maxPos0, "maxPos has an incorrect result");
    assert_equal_sequence(H, H0, m*n, "H has an incorrect result");
    assert_equal_sequence(P, P0, m*n, "P has an incorrect result");
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << std::endl;
    exit(1);
  }

  std::cout << elapsedtime << std::endl;
  return 0;
} 


