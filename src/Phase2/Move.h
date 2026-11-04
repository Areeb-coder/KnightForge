#ifndef MOVE_H
#define MOVE_H

#include "../Phase1/Piece.h"
#include "Square.h"

#include <string>

// ----------------------------------------
// Chess Move
// ----------------------------------------

struct Move {

    int fromRow;
    int fromCol;

    int toRow;
    int toCol;

    PieceType promotion;
};

// ----------------------------------------
// Function declarations
// ----------------------------------------

Square parseSquare(const std::string& square);

PieceType parsePromotion(char piece);

Move parseMove(const std::string& input);

#endif
