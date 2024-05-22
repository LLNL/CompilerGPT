#undef NDEBUG
#include<assert.h>
int main(){
    assert (make_palindrome("") == "");
    assert (make_palindrome("x") == "x");
     assert (make_palindrome("xyz") == "xyzyx");
     assert (make_palindrome("xyx") == "xyx") ;
     assert (make_palindrome("jerry") == "jerryrrej");
}


