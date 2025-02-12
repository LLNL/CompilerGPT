#include "utility.h"

auto prefixScan(std::vector<long double> elems)
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
