#include<stdio.h>
#include<math.h>
#include<vector>
using namespace std;
#include<algorithm>
#include<stdlib.h>
vector<int> compare(vector<int> game,vector<int> guess){
    vector<int> out;
    for (int i=0;i<game.size();i++)
    out.push_back(abs(game[i]-guess[i]));
    return out;
}
