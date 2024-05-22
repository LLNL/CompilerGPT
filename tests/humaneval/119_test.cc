#undef NDEBUG
#include<assert.h>
int main(){
    assert (match_parens({"()(", ")"}) == "Yes");
    assert (match_parens({")", ")"}) == "No");
    assert (match_parens({"(()(())", "())())"}) == "No");
    assert (match_parens({")())", "(()()("}) == "Yes");
    assert (match_parens({"(())))", "(()())(("}) == "Yes");
    assert (match_parens({"()", "())"}) == "No");
    assert (match_parens({"(()(", "()))()"}) == "Yes");
    assert (match_parens({"((((", "((())"}) == "No");
    assert (match_parens({")(()", "(()("}) == "No");
    assert (match_parens({")(", ")("}) == "No");
    assert (match_parens({"(", ")"}) == "Yes");
    assert (match_parens({")", "("}) == "Yes" );
}
