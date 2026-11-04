#include "Chess.h"
#include <cassert>
#include <iostream>

using namespace chess;

static bool containsMove(Board& b, const std::string& uci) {
    for (const Move& m : b.generateLegalMoves(b.sideToMove())) {
        if (moveToUci(m) == uci) return true;
    }
    return false;
}

static void testStartingMoves() {
    Board b;
    assert(b.generateLegalMoves(Color::White).size() == 20);
}

static void testPinnedPieceCannotExposeKing() {
    Board b;
    std::string error;
    assert(b.fromFEN("4r1k1/8/8/8/8/8/4R3/4K3 w - - 0 1", &error));
    assert(!containsMove(b, "e2f2"));
}

static void testCastling() {
    Board b;
    std::string error;
    assert(b.fromFEN("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", &error));
    assert(containsMove(b, "e1g1"));
    assert(containsMove(b, "e1c1"));

    Move m;
    assert(b.parseUserMove("e1g1", m));
    UndoState u;
    assert(b.makeMove(m, u));
    assert(b.squares()[stringToSquare("g1")].type == PieceType::King);
    assert(b.squares()[stringToSquare("f1")].type == PieceType::Rook);
}

static void testCastlingThroughCheckForbidden() {
    Board b;
    std::string error;
    assert(b.fromFEN("r3k2r/8/8/8/8/5r2/8/R3K2R w KQkq - 0 1", &error));
    assert(!containsMove(b, "e1g1"));
}

static void testEnPassant() {
    Board b;
    std::string error;
    assert(b.fromFEN("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", &error));
    assert(containsMove(b, "e5d6"));

    Move m;
    assert(b.parseUserMove("e5d6", m));
    assert(m.isEnPassant);
    UndoState u;
    assert(b.makeMove(m, u));
    assert(b.squares()[stringToSquare("d6")].type == PieceType::Pawn);
    assert(b.squares()[stringToSquare("d5")].type == PieceType::None);
}

static void testPromotion() {
    Board b;
    std::string error;
    assert(b.fromFEN("7k/4P3/8/8/8/8/8/4K3 w - - 0 1", &error));
    assert(containsMove(b, "e7e8q"));
    assert(containsMove(b, "e7e8r"));
    assert(containsMove(b, "e7e8b"));
    assert(containsMove(b, "e7e8n"));
}

static void testCheckmate() {
    Board b;
    std::string error;
    assert(b.fromFEN("7k/6Q1/6K1/8/8/8/8/8 b - - 0 1", &error));
    assert(b.isInCheck(Color::Black));
    assert(b.generateLegalMoves(Color::Black).empty());
}

static void testStalemate() {
    Board b;
    std::string error;
    assert(b.fromFEN("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1", &error));
    assert(!b.isInCheck(Color::Black));
    assert(b.generateLegalMoves(Color::Black).empty());
}

static void testFENRoundTrip() {
    Board b;
    std::string error;
    const std::string fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1";
    assert(b.fromFEN(fen, &error));
    assert(b.toFEN() == fen);
}

static void testMakeUndo() {
    Board b;
    Move m;
    assert(b.parseUserMove("e2e4", m));
    UndoState u;
    auto before = b.toFEN();
    assert(b.makeMove(m, u));
    b.undoMove(m, u);
    assert(b.toFEN() == before);
}

int main() {
    testStartingMoves();
    testPinnedPieceCannotExposeKing();
    testCastling();
    testCastlingThroughCheckForbidden();
    testEnPassant();
    testPromotion();
    testCheckmate();
    testStalemate();
    testFENRoundTrip();
    testMakeUndo();
    std::cout << "All Phase 3 tests passed.\n";
    return 0;
}
