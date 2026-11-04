#ifndef PIECE_H
#define PIECE_H

// ----------------------------------------
// Piece Types
// ----------------------------------------

enum class PieceType {
    NONE,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

// ----------------------------------------
// Piece Colors
// ----------------------------------------

enum class Color {
    NONE,
    WHITE,
    BLACK
};

// ----------------------------------------
// Chess Piece
// ----------------------------------------

struct Piece {
    PieceType type;
    Color color;
};

#endif
