// $(CXX) -Wall -Wextra -O3 -march=native -DNDEBUG=1 -fopenmp perf.cc utility.cc -o perf

#include <iostream>
#include <chrono>
#include <numeric>

#include "utility.h"


auto originalPrefixScan(std::vector<long double> elems)
  -> std::decay<decltype(elems)>::type
{
  using vector_type = std::decay<decltype(elems)>::type;

  // Create a copy of the input, so that we don't mutate the original input
  const int   maxlen = int(elems.size());
  vector_type res    = elems;

  for (int dist=1; dist < maxlen; dist<<=1)
  {
    vector_type tmp = res;

    // Compute the sum of pairs of values
    #pragma omp parallel for
    for (int i=dist; i < maxlen; ++i)
    {
      res[i] += tmp[i-dist];
    }
  }

  return res;
}



void init(std::vector<long double>& elems)
{
  std::iota(elems.begin(), elems.end(), (-elems.size()) / 2);
}


void fail(const char* msg)
{
  throw std::runtime_error(msg);
}

void assert_equal(int lhs, int rhs, const char* msg)
{
  if (lhs != rhs)
    throw std::runtime_error(msg);
}

void assert_true(bool b, const char* msg)
{
  if (!b)
    throw std::runtime_error(msg);
}

template <class Container>
void assert_equal_size(const Container& lhs, const Container& rhs)
{
  assert_equal(lhs.size(), rhs.size(), "number of elements mismatch");
}


template <class T>
void assert_equal(const std::vector<T>& lhs, const std::vector<T>& rhs)
{
  assert_equal_size(lhs, rhs);
  assert_true(std::equal(lhs.begin(), lhs.end(), rhs.begin()), "regression test failed");
}



int main()
{
  using time_point = std::chrono::time_point<std::chrono::system_clock>;

  int N = 55555584;

  std::vector<long double> elems(N, 0);

  init(elems);

  time_point   starttime   = std::chrono::system_clock::now();
  elems = prefixScan(std::move(elems));
  time_point   endtime     = std::chrono::system_clock::now();
  int          elapsedtime = std::chrono::duration_cast<std::chrono::milliseconds>(endtime-starttime).count();

  std::vector<long double> check(N, 0);

  init(check);
  check = originalPrefixScan(std::move(check));

  try
  {
    assert_equal(elems, check);
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << std::endl;
    exit(1);
  }

  std::cout << elapsedtime << std::endl;
  return 0;
}


