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
    assert (issame(by_length({2, 1, 1, 4, 5, 8, 2, 3}) , {"Eight", "Five", "Four", "Three", "Two", "Two", "One", "One"}));
    assert (issame(by_length({}) , {}));
    assert (issame(by_length({1, -1 , 55}) , {"One"}));
    assert (issame(by_length({1, -1, 3, 2}) , {"Three", "Two", "One"}));
    assert (issame(by_length({9, 4, 8}) , {"Nine", "Eight", "Four"}));
}
