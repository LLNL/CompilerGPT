#include <vector>
#include <type_traits>

auto prefixScan(std::vector<long double> elems)
  -> std::decay<decltype(elems)>::type;
