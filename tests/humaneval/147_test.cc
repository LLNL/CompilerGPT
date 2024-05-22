#undef NDEBUG
#include<assert.h>
int main(){
    assert (get_matrix_triples(5) == 1);
    assert (get_matrix_triples(6) == 4);
    assert (get_matrix_triples(10) == 36);
    assert (get_matrix_triples(100) == 53361);
}
