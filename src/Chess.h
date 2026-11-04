#pragma once

#include "Phase1/Piece.h"
#include "Phase2/Move.h"

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

struct Position {
    Piece board[8][8]{};
    Color sideToMove = Color::WHITE;
    bool whiteKingSideCastle = true;
    bool whiteQueenSideCastle = true;
    bool blackKingSideCastle = true;
    bool blackQueenSideCastle = true;
    int enPassantRow = -1;
    int enPassantCol = -1;
    int halfmoveClock = 0;
    int fullmoveNumber = 1;
};

struct StateInfo {
    Piece captured{};
    Piece moved{};
    bool wks, wqs, bks, bqs;
    int epRow, epCol;
    int halfmoveClock, fullmoveNumber;
    Color sideToMove;
};

struct SearchResult {
    Move move{};
    int score = 0;
    int depth = 0;
    std::uint64_t nodes = 0;
};

Position startPosition();
std::string positionToFEN(const Position& p);
bool positionFromFEN(const std::string& fen, Position& p);

bool isInside(int r, int c);
int findKing(const Position& p, Color side);
bool isSquareAttacked(const Position& p, int r, int c, Color bySide);
bool isInCheck(const Position& p, Color side);

std::vector<Move> generatePseudoLegalMoves(const Position& p);
std::vector<Move> generateLegalMoves(Position& p);

void makeMove(Position& p, const Move& m, StateInfo& state);
void undoMove(Position& p, const Move& m, const StateInfo& state);

std::uint64_t positionKey(const Position& p);
std::uint64_t perft(Position& p, int depth);

int evaluate(const Position& p);
SearchResult searchBestMove(Position& p, int maxDepth, int timeLimitMs);

std::string moveToString(const Move& m);
std::string colorName(Color c);
std::string pieceName(PieceType t);
