#include "../src/Chess.h"
#include <cassert>
#include <iostream>
#include <string>

int main() {
    {
        Position p = startPosition();
        Move m{};
        assert(parseSANMove(p, "e4", m));
        assert(moveToSAN(p, m) == "e4");
        StateInfo st{}; makeMove(p, m, st);

        assert(parseSANMove(p, "Nf6", m));
        assert(moveToSAN(p, m) == "Nf6");
    }

    {
        Position p{};
        assert(positionFromFEN("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", p));
        auto legal = generateLegalMoves(p);
        bool kingSide=false, queenSide=false;
        for (const auto& m : legal) {
            std::string san = moveToSAN(p, m);
            if (san == "O-O") kingSide=true;
            if (san == "O-O-O") queenSide=true;
        }
        assert(kingSide && queenSide);
        Move m{}; assert(parseSANMove(p, "O-O", m));
    }

    {
        Position p{};
        assert(positionFromFEN("k7/4P3/8/8/8/8/8/4K3 w - - 0 1", p));
        Move m{}; assert(parseSANMove(p, "e8=Q", m));
        assert(moveToSAN(p, m) == "e8=Q+");
    }

    std::cout << "SAN tests passed.\n";
    return 0;
}
