#undef NDEBUG
#include<assert.h>
bool issame(vector<string> a,vector<string>b){
    if (a.size()!=b.size()) return false;
    for (int i=0;i<a.size();i++)
    {
    if (a[i]!=b[i]) return false;
    }
    return true;
}
int main(){
    assert (issame(bf("Jupiter", "Neptune") , {"Saturn", "Uranus"}));
    assert (issame(bf("Earth", "Mercury") , {"Venus",}));
    assert (issame(bf("Mercury", "Uranus") , {"Venus", "Earth", "Mars", "Jupiter", "Saturn"}));
    assert (issame(bf("Neptune", "Venus") , {"Earth", "Mars", "Jupiter", "Saturn", "Uranus"}));
    assert (issame(bf("Earth", "Earth") , {}));
    assert (issame(bf("Mars", "Earth") , {}));
    assert (issame(bf("Jupiter", "Makemake") , {}));
}
