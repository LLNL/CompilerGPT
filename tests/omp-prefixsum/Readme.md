= Tests OMP Prefix Sum algorithm =

C++ Algorithm modified from C version at: https://github.com/robfarr/openmp-prefix-sum/blob/master/main.c

Tests optimizations for OpenMP and C++ code prefixSum in utility.cc.

Ideas for optimizations include:
  Hoisting tmp - vector generation outside the loop.
  Using OpenMP forloop to initialize tmp vector
  Enlargen the parallel region


