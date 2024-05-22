#include<stdio.h>
#include<math.h>
#include<vector>
#include<algorithm>
using namespace std;
#include<stdlib.h>
vector<int> minPath(vector<vector<int>> grid, int k){
    int i,j,x,y,min;
    for (i=0;i<grid.size();i++)
        for (j=0;j<grid[i].size();j++)
            if (grid[i][j]==1) {
                x=i;y=j;
            }
    min=grid.size()*grid.size();
    if (x>0 and grid[x-1][y]<min) min=grid[x-1][y];
    if (x<grid.size()-1 and grid[x+1][y]<min) min=grid[x+1][y];
    if (y>0 and grid[x][y-1]<min) min=grid[x][y-1];
    if (y<grid.size()-1 and grid[x][y+1]<min) min=grid[x][y+1];
    vector<int> out={};
    for (i=0;i<k;i++)
    if (i%2==0) out.push_back(1);
    else out.push_back(min);
    return out;
}
