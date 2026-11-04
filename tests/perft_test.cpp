#include "Chess.h"
#include <cassert>
#include <iostream>
int main(){
    Position p=startPosition();
    assert(perft(p,1)==20);
    assert(perft(p,2)==400);
    assert(perft(p,3)==8902);
    assert(perft(p,4)==197281);
    std::cout<<"Perft tests passed.\n";
    return 0;
}
