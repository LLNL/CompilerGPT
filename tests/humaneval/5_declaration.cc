#include<stdio.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<math.h>
#include<stdlib.h>
vector<int> intersperse(vector<int> numbers, int delimeter){ 
    vector<int> out={};
    if (numbers.size()>0) out.push_back(numbers[0]);
    for (int i=1;i<numbers.size();i++)
    {
        out.push_back(delimeter);
        out.push_back(numbers[i]);

    }
    return out;
}
