
// constants.h

#pragma once

const int matchScore = 2; 
const int missmatchScore = -1; 
const int gapScore = -2; 
const int NONE = 0; 
const int DIAGONAL = 1; 
const int UP = 2; 
const int LEFT = 3; 

// Declare the compute function
void compute(const char *a, const char *b, int *H, int *P, long long n, long long m, long long &maxPos);
