#include "Piece.h"

Piece::Piece() {
    type = PieceType::None;
    isWhite = true;
}

Piece::Piece(PieceType type, bool isWhite) {
    this->type = type;
    this->isWhite = isWhite;
}
