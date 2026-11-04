#include "Move.h"

// ----------------------------------------
// Convert chess square to board coordinates
//
// Example:
//
// e2
// row = 6
// col = 4
//
// Your board uses:
//
// row 0 = rank 8
// row 1 = rank 7
// ...
// row 6 = rank 2
// row 7 = rank 1
// ----------------------------------------

Square parseSquare(const std::string& square) {

    Square result;

    // Convert file
    // a = 0
    // b = 1
    // ...
    // h = 7

    result.col = square[0] - 'a';

    // Convert rank
    // 8 = 0
    // 7 = 1
    // ...
    // 1 = 7

    result.row = '8' - square[1];

    return result;
}


// ----------------------------------------
// Convert promotion character
//
// q = Queen
// r = Rook
// b = Bishop
// n = Knight
// ----------------------------------------

PieceType parsePromotion(char piece) {

    switch (piece) {

        case 'q':
        case 'Q':
            return PieceType::QUEEN;

        case 'r':
        case 'R':
            return PieceType::ROOK;

        case 'b':
        case 'B':
            return PieceType::BISHOP;

        case 'n':
        case 'N':
            return PieceType::KNIGHT;

        default:
            return PieceType::NONE;
    }
}


// ----------------------------------------
// Convert user input into a Move
//
// Example:
//
// e2e4
//
// becomes:
//
// from = e2
// to   = e4
// ----------------------------------------

Move parseMove(const std::string& input) {

    Move move;

    // Get starting square
    Square from = parseSquare(
        input.substr(0, 2)
    );

    // Get destination square
    Square to = parseSquare(
        input.substr(2, 2)
    );

    // Store starting position
    move.fromRow = from.row;
    move.fromCol = from.col;

    // Store destination position
    move.toRow = to.row;
    move.toCol = to.col;

    // Normal move has no promotion
    move.promotion = PieceType::NONE;

    // Check for promotion
    //
    // Example:
    // e7e8q

    if (input.length() == 5) {

        move.promotion =
            parsePromotion(input[4]);
    }

    return move;
}
