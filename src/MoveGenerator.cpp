#include "Chess.h"

namespace chess {

static void addPromotionMoves(int from, int to, bool capture, std::vector<Move>& out, bool enPassant = false, bool castle = false) {
    (void)capture;
    (void)enPassant;
    (void)castle;
    out.push_back({from,to,PieceType::Queen,false,false});
    out.push_back({from,to,PieceType::Rook,false,false});
    out.push_back({from,to,PieceType::Bishop,false,false});
    out.push_back({from,to,PieceType::Knight,false,false});
}

void MoveGenerator::addPawnMoves(const Board& b, Color side, int sq, std::vector<Move>& out) {
    const auto& board = b.squares();
    int r = rowOf(sq), c = colOf(sq);
    int dir = (side == Color::White) ? 1 : -1;
    int startRank = (side == Color::White) ? 1 : 6;
    int promotionRank = (side == Color::White) ? 7 : 0;

    int oneR = r + dir;
    if (onBoard(oneR, c)) {
        int one = makeSquare(oneR,c);
        if (board[one].type == PieceType::None) {
            if (oneR == promotionRank) addPromotionMoves(sq, one, false, out);
            else out.push_back({sq,one});

            int twoR = r + 2*dir;
            if (r == startRank && onBoard(twoR,c)) {
                int two = makeSquare(twoR,c);
                if (board[two].type == PieceType::None) out.push_back({sq,two});
            }
        }
    }

    for (int dc : {-1,1}) {
        int nr = r + dir, nc = c + dc;
        if (!onBoard(nr,nc)) continue;
        int to = makeSquare(nr,nc);
        if (board[to].color == opposite(side)) {
            if (nr == promotionRank) addPromotionMoves(sq,to,true,out);
            else out.push_back({sq,to});
        }
        if (to == b.getEnPassantSquare()) {
            Move m{sq,to,PieceType::None,true,false};
            out.push_back(m);
        }
    }
}

void MoveGenerator::addKnightMoves(const Board& b, Color side, int sq, std::vector<Move>& out) {
    const auto& board = b.squares();
    int r = rowOf(sq), c = colOf(sq);
    static const int dirs[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for (const auto& d : dirs) {
        int nr = r + d[0], nc = c + d[1];
        if (!onBoard(nr,nc)) continue;
        int to = makeSquare(nr,nc);
        if (board[to].color != side) out.push_back({sq,to});
    }
}

void MoveGenerator::addSlidingMoves(const Board& b, Color side, int sq, const std::vector<std::pair<int,int>>& dirs, std::vector<Move>& out) {
    const auto& board = b.squares();
    int r = rowOf(sq), c = colOf(sq);
    for (auto [dr,dc] : dirs) {
        int nr = r + dr, nc = c + dc;
        while (onBoard(nr,nc)) {
            int to = makeSquare(nr,nc);
            if (board[to].type == PieceType::None) {
                out.push_back({sq,to});
            } else {
                if (board[to].color != side) out.push_back({sq,to});
                break;
            }
            nr += dr; nc += dc;
        }
    }
}

void MoveGenerator::addKingMoves(const Board& b, Color side, int sq, std::vector<Move>& out) {
    const auto& board = b.squares();
    int r = rowOf(sq), c = colOf(sq);
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int nr = r + dr, nc = c + dc;
            if (!onBoard(nr,nc)) continue;
            int to = makeSquare(nr,nc);
            if (board[to].color != side) out.push_back({sq,to});
        }
    }

    // Castling. We check attack conditions here using the board state before the move.
    if (side == Color::White && sq == stringToSquare("e1") && !b.isInCheck(Color::White)) {
        // Kingside
        if (b.canCastleKingSide(Color::White) &&
            board[stringToSquare("h1")].type == PieceType::Rook &&
            board[stringToSquare("h1")].color == Color::White &&
            board[stringToSquare("f1")].type == PieceType::None &&
            board[stringToSquare("g1")].type == PieceType::None &&
            !b.isSquareAttacked(stringToSquare("f1"), Color::Black) &&
            !b.isSquareAttacked(stringToSquare("g1"), Color::Black)) {
            // Castling rights inferred from initial rook/king state; Board's rights are private.
            // The final legal-move filter also ensures the move itself is legal.
            // We accept only if a rook is still on h1 and path is clear.
            out.push_back({sq,stringToSquare("g1"),PieceType::None,false,true});
        }
        // Queenside
        if (b.canCastleQueenSide(Color::White) &&
            board[stringToSquare("a1")].type == PieceType::Rook &&
            board[stringToSquare("a1")].color == Color::White &&
            board[stringToSquare("b1")].type == PieceType::None &&
            board[stringToSquare("c1")].type == PieceType::None &&
            board[stringToSquare("d1")].type == PieceType::None &&
            !b.isSquareAttacked(stringToSquare("d1"), Color::Black) &&
            !b.isSquareAttacked(stringToSquare("c1"), Color::Black)) {
            out.push_back({sq,stringToSquare("c1"),PieceType::None,false,true});
        }
    }
    if (side == Color::Black && sq == stringToSquare("e8") && !b.isInCheck(Color::Black)) {
        if (b.canCastleKingSide(Color::Black) &&
            board[stringToSquare("h8")].type == PieceType::Rook &&
            board[stringToSquare("h8")].color == Color::Black &&
            board[stringToSquare("f8")].type == PieceType::None &&
            board[stringToSquare("g8")].type == PieceType::None &&
            !b.isSquareAttacked(stringToSquare("f8"), Color::White) &&
            !b.isSquareAttacked(stringToSquare("g8"), Color::White)) {
            out.push_back({sq,stringToSquare("g8"),PieceType::None,false,true});
        }
        if (b.canCastleQueenSide(Color::Black) &&
            board[stringToSquare("a8")].type == PieceType::Rook &&
            board[stringToSquare("a8")].color == Color::Black &&
            board[stringToSquare("b8")].type == PieceType::None &&
            board[stringToSquare("c8")].type == PieceType::None &&
            board[stringToSquare("d8")].type == PieceType::None &&
            !b.isSquareAttacked(stringToSquare("d8"), Color::White) &&
            !b.isSquareAttacked(stringToSquare("c8"), Color::White)) {
            out.push_back({sq,stringToSquare("c8"),PieceType::None,false,true});
        }
    }
}

} // namespace chess
