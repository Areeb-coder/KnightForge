#include "Chess.h"

namespace chess {

Board::Board() { reset(); }

void Board::reset() {
    board.fill({});
    const std::string back = "RNBQKBNR";
    const std::string pawn = "PPPPPPPP";
    for (int c = 0; c < 8; ++c) {
        board[makeSquare(0,c)] = charToPiece(back[c]);
        board[makeSquare(1,c)] = charToPiece(pawn[c]);
        board[makeSquare(6,c)] = charToPiece(static_cast<char>(std::tolower(pawn[c])));
        board[makeSquare(7,c)] = charToPiece(static_cast<char>(std::tolower(back[c])));
    }
    side = Color::White;
    whiteKingCastle = whiteQueenCastle = blackKingCastle = blackQueenCastle = true;
    enPassantSquare = -1;
    halfmoveClock = 0;
    fullmoveNumber = 1;
    repetitionHistory.clear();
    repetitionHistory.push_back(positionHash());
}

void Board::print(std::ostream& out) const {
    out << "\n    +------------------------+\n";
    for (int r = 7; r >= 0; --r) {
        out << ' ' << (r + 1) << "  |";
        for (int c = 0; c < 8; ++c) {
            out << ' ' << pieceToChar(board[makeSquare(r,c)]) << ' ';
        }
        out << "|\n";
    }
    out << "    +------------------------+\n";
    out << "       a  b  c  d  e  f  g  h\n\n";
}

void Board::setError(std::string* error, const std::string& msg) const {
    if (error) *error = msg;
}

bool Board::fromFEN(const std::string& fen, std::string* error) {
    std::istringstream iss(fen);
    std::string placement, active, castling, ep;
    int half = 0, full = 1;
    if (!(iss >> placement >> active >> castling >> ep >> half >> full)) {
        setError(error, "FEN must contain 6 fields.");
        return false;
    }

    std::array<Piece,64> newBoard{};
    std::vector<std::string> ranks;
    std::stringstream ps(placement);
    std::string rank;
    while (std::getline(ps, rank, '/')) ranks.push_back(rank);
    if (ranks.size() != 8) {
        setError(error, "FEN board must have 8 ranks.");
        return false;
    }

    for (int fenRank = 0; fenRank < 8; ++fenRank) {
        int file = 0;
        for (char ch : ranks[fenRank]) {
            if (std::isdigit(static_cast<unsigned char>(ch))) {
                file += ch - '0';
            } else {
                if (file >= 8) {
                    setError(error, "Invalid FEN rank.");
                    return false;
                }
                Piece p = charToPiece(ch);
                if (p.type == PieceType::None) {
                    setError(error, "Invalid FEN piece character.");
                    return false;
                }
                newBoard[makeSquare(7 - fenRank, file)] = p;
                ++file;
            }
        }
        if (file != 8) {
            setError(error, "Each FEN rank must describe exactly 8 squares.");
            return false;
        }
    }

    if (active == "w") side = Color::White;
    else if (active == "b") side = Color::Black;
    else {
        setError(error, "Invalid FEN active color.");
        return false;
    }

    whiteKingCastle = castling.find('K') != std::string::npos;
    whiteQueenCastle = castling.find('Q') != std::string::npos;
    blackKingCastle = castling.find('k') != std::string::npos;
    blackQueenCastle = castling.find('q') != std::string::npos;

    if (ep == "-") enPassantSquare = -1;
    else {
        enPassantSquare = stringToSquare(ep);
        if (enPassantSquare < 0) {
            setError(error, "Invalid FEN en-passant square.");
            return false;
        }
    }

    if (half < 0 || full <= 0) {
        setError(error, "Invalid FEN move counters.");
        return false;
    }

    board = newBoard;
    halfmoveClock = half;
    fullmoveNumber = full;
    repetitionHistory.clear();
    repetitionHistory.push_back(positionHash());
    return true;
}

std::string Board::toFEN() const {
    std::ostringstream out;
    for (int r = 7; r >= 0; --r) {
        int empty = 0;
        for (int c = 0; c < 8; ++c) {
            Piece p = board[makeSquare(r,c)];
            if (p.type == PieceType::None) {
                ++empty;
            } else {
                if (empty) { out << empty; empty = 0; }
                out << pieceToChar(p);
            }
        }
        if (empty) out << empty;
        if (r > 0) out << '/';
    }
    out << ' ' << (side == Color::White ? 'w' : 'b') << ' ';
    std::string rights;
    if (whiteKingCastle) rights += 'K';
    if (whiteQueenCastle) rights += 'Q';
    if (blackKingCastle) rights += 'k';
    if (blackQueenCastle) rights += 'q';
    out << (rights.empty() ? "-" : rights) << ' ';
    out << (enPassantSquare < 0 ? "-" : squareToString(enPassantSquare)) << ' ';
    out << halfmoveClock << ' ' << fullmoveNumber;
    return out.str();
}

uint64_t Board::positionHash() const {
    // Deterministic FNV-1a style hash. Adequate for this basic engine's TT.
    uint64_t h = 1469598103934665603ULL;
    auto mix = [&](uint64_t x) {
        h ^= x;
        h *= 1099511628211ULL;
    };
    for (int sq = 0; sq < 64; ++sq) {
        const Piece p = board[sq];
        uint64_t code = static_cast<uint64_t>(p.type) * 4ULL + static_cast<uint64_t>(p.color);
        mix(code + static_cast<uint64_t>(sq + 1) * 0x9e3779b97f4a7c15ULL);
    }
    mix(static_cast<uint64_t>(side));
    mix(whiteKingCastle ? 1ULL : 0ULL);
    mix(whiteQueenCastle ? 2ULL : 0ULL);
    mix(blackKingCastle ? 4ULL : 0ULL);
    mix(blackQueenCastle ? 8ULL : 0ULL);
    mix(static_cast<uint64_t>(enPassantSquare + 1));
    return h;
}

int Board::findKing(Color c) const {
    for (int sq = 0; sq < 64; ++sq) {
        if (board[sq].type == PieceType::King && board[sq].color == c) return sq;
    }
    return -1;
}

bool Board::isSquareAttacked(int sq, Color by) const {
    if (sq < 0 || sq >= 64) return false;
    const int r = rowOf(sq), c = colOf(sq);

    // Pawn attacks.
    const int pawnDir = (by == Color::White) ? -1 : 1;
    const int pawnRow = r + pawnDir;
    for (int dc : {-1, 1}) {
        if (onBoard(pawnRow, c + dc)) {
            Piece p = board[makeSquare(pawnRow, c + dc)];
            if (p.color == by && p.type == PieceType::Pawn) return true;
        }
    }

    // Knight attacks.
    static const int knightDirs[8][2] = {
        {-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}
    };
    for (const auto& d : knightDirs) {
        int nr = r + d[0], nc = c + d[1];
        if (!onBoard(nr,nc)) continue;
        Piece p = board[makeSquare(nr,nc)];
        if (p.color == by && p.type == PieceType::Knight) return true;
    }

    // Sliding attacks.
    static const int bishopDirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    for (const auto& d : bishopDirs) {
        int nr = r + d[0], nc = c + d[1];
        while (onBoard(nr,nc)) {
            Piece p = board[makeSquare(nr,nc)];
            if (p.type != PieceType::None) {
                if (p.color == by && (p.type == PieceType::Bishop || p.type == PieceType::Queen)) return true;
                break;
            }
            nr += d[0]; nc += d[1];
        }
    }
    static const int rookDirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    for (const auto& d : rookDirs) {
        int nr = r + d[0], nc = c + d[1];
        while (onBoard(nr,nc)) {
            Piece p = board[makeSquare(nr,nc)];
            if (p.type != PieceType::None) {
                if (p.color == by && (p.type == PieceType::Rook || p.type == PieceType::Queen)) return true;
                break;
            }
            nr += d[0]; nc += d[1];
        }
    }

    // King attacks.
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int nr = r + dr, nc = c + dc;
            if (!onBoard(nr,nc)) continue;
            Piece p = board[makeSquare(nr,nc)];
            if (p.color == by && p.type == PieceType::King) return true;
        }
    }

    return false;
}

bool Board::isInCheck(Color c) const {
    int king = findKing(c);
    if (king < 0) return true;
    return isSquareAttacked(king, opposite(c));
}

std::vector<Move> Board::generatePseudoLegalMoves(Color movingSide) const {
    std::vector<Move> moves;
    for (int sq = 0; sq < 64; ++sq) {
        const Piece p = board[sq];
        if (p.color != movingSide) continue;
        switch (p.type) {
            case PieceType::Pawn: MoveGenerator::addPawnMoves(*this, movingSide, sq, moves); break;
            case PieceType::Knight: MoveGenerator::addKnightMoves(*this, movingSide, sq, moves); break;
            case PieceType::Bishop: MoveGenerator::addSlidingMoves(*this, movingSide, sq, {{1,1},{1,-1},{-1,1},{-1,-1}}, moves); break;
            case PieceType::Rook: MoveGenerator::addSlidingMoves(*this, movingSide, sq, {{1,0},{-1,0},{0,1},{0,-1}}, moves); break;
            case PieceType::Queen: MoveGenerator::addSlidingMoves(*this, movingSide, sq, {{1,1},{1,-1},{-1,1},{-1,-1},{1,0},{-1,0},{0,1},{0,-1}}, moves); break;
            case PieceType::King: MoveGenerator::addKingMoves(*this, movingSide, sq, moves); break;
            default: break;
        }
    }
    return moves;
}

bool Board::makeMove(const Move& move, UndoState& undo) {
    if (move.from < 0 || move.to < 0 || move.from >= 64 || move.to >= 64) return false;
    Piece moving = board[move.from];
    if (moving.color != side) return false;

    undo.moved = moving;
    undo.captured = board[move.to];
    undo.capturedSquare = move.to;
    undo.previousEnPassant = enPassantSquare;
    undo.wK = whiteKingCastle; undo.wQ = whiteQueenCastle;
    undo.bK = blackKingCastle; undo.bQ = blackQueenCastle;
    undo.halfmove = halfmoveClock; undo.fullmove = fullmoveNumber;
    undo.hash = positionHash();

    board[move.to] = board[move.from];
    board[move.from] = {};

    // En passant capture.
    if (move.isEnPassant) {
        int capSq = move.to + (moving.color == Color::White ? -8 : 8);
        undo.capturedSquare = capSq;
        undo.captured = board[capSq];
        board[capSq] = {};
    }

    // Promotion.
    if (moving.type == PieceType::Pawn && move.promotion != PieceType::None) {
        board[move.to].type = move.promotion;
    }

    // Castling rook move.
    if (move.isCastle) {
        if (move.to == stringToSquare("g1")) {
            board[stringToSquare("f1")] = board[stringToSquare("h1")];
            board[stringToSquare("h1")] = {};
        } else if (move.to == stringToSquare("c1")) {
            board[stringToSquare("d1")] = board[stringToSquare("a1")];
            board[stringToSquare("a1")] = {};
        } else if (move.to == stringToSquare("g8")) {
            board[stringToSquare("f8")] = board[stringToSquare("h8")];
            board[stringToSquare("h8")] = {};
        } else if (move.to == stringToSquare("c8")) {
            board[stringToSquare("d8")] = board[stringToSquare("a8")];
            board[stringToSquare("a8")] = {};
        }
    }

    updateCastlingRightsForMove(move, undo);

    enPassantSquare = -1;
    if (moving.type == PieceType::Pawn && std::abs(move.to - move.from) == 16) {
        enPassantSquare = (move.from + move.to) / 2;
    }

    if (moving.type == PieceType::Pawn || undo.captured.type != PieceType::None) halfmoveClock = 0;
    else ++halfmoveClock;
    if (side == Color::Black) ++fullmoveNumber;
    side = opposite(side);
    return true;
}

void Board::updateCastlingRightsForMove(const Move& move, const UndoState& undo) {
    if (undo.moved.type == PieceType::King) {
        if (undo.moved.color == Color::White) whiteKingCastle = whiteQueenCastle = false;
        else blackKingCastle = blackQueenCastle = false;
    }
    if (undo.moved.type == PieceType::Rook) {
        if (move.from == stringToSquare("a1")) whiteQueenCastle = false;
        if (move.from == stringToSquare("h1")) whiteKingCastle = false;
        if (move.from == stringToSquare("a8")) blackQueenCastle = false;
        if (move.from == stringToSquare("h8")) blackKingCastle = false;
    }
    if (undo.captured.type == PieceType::Rook) {
        if (undo.capturedSquare == stringToSquare("a1")) whiteQueenCastle = false;
        if (undo.capturedSquare == stringToSquare("h1")) whiteKingCastle = false;
        if (undo.capturedSquare == stringToSquare("a8")) blackQueenCastle = false;
        if (undo.capturedSquare == stringToSquare("h8")) blackKingCastle = false;
    }
}

void Board::undoMove(const Move& move, const UndoState& undo) {
    side = opposite(side);
    if (side == Color::Black) --fullmoveNumber;
    whiteKingCastle = undo.wK; whiteQueenCastle = undo.wQ;
    blackKingCastle = undo.bK; blackQueenCastle = undo.bQ;
    enPassantSquare = undo.previousEnPassant;
    halfmoveClock = undo.halfmove;
    fullmoveNumber = undo.fullmove;

    board[move.from] = undo.moved;
    board[move.to] = {};

    if (move.isEnPassant) {
        board[undo.capturedSquare] = undo.captured;
    } else {
        board[move.to] = undo.captured;
    }

    if (move.isCastle) {
        if (move.to == stringToSquare("g1")) {
            board[stringToSquare("h1")] = board[stringToSquare("f1")];
            board[stringToSquare("f1")] = {};
        } else if (move.to == stringToSquare("c1")) {
            board[stringToSquare("a1")] = board[stringToSquare("d1")];
            board[stringToSquare("d1")] = {};
        } else if (move.to == stringToSquare("g8")) {
            board[stringToSquare("h8")] = board[stringToSquare("f8")];
            board[stringToSquare("f8")] = {};
        } else if (move.to == stringToSquare("c8")) {
            board[stringToSquare("a8")] = board[stringToSquare("d8")];
            board[stringToSquare("d8")] = {};
        }
    }
}

std::vector<Move> Board::generateLegalMoves(Color movingSide) {
    std::vector<Move> legal;
    auto pseudo = generatePseudoLegalMoves(movingSide);
    legal.reserve(pseudo.size());
    for (const Move& move : pseudo) {
        UndoState undo;
        Color oldSide = side;
        if (oldSide != movingSide) continue;
        if (!makeMove(move, undo)) continue;
        bool illegal = isInCheck(movingSide);
        undoMove(move, undo);
        if (!illegal) legal.push_back(move);
    }
    return legal;
}

bool Board::parseUserMove(const std::string& input, Move& out) const {
    std::string s;
    for (char ch : input) {
        if (!std::isspace(static_cast<unsigned char>(ch))) s += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    if (s.size() < 4 || s.size() > 5) return false;
    int from = stringToSquare(s.substr(0,2));
    int to = stringToSquare(s.substr(2,2));
    if (from < 0 || to < 0) return false;
    PieceType promo = PieceType::None;
    if (s.size() == 5) {
        switch (s[4]) {
            case 'q': promo = PieceType::Queen; break;
            case 'r': promo = PieceType::Rook; break;
            case 'b': promo = PieceType::Bishop; break;
            case 'n': promo = PieceType::Knight; break;
            default: return false;
        }
    }
    out = Move{from,to,promo,false,false};
    // Match against generated legal moves so special flags are filled correctly.
    auto legal = const_cast<Board*>(this)->generateLegalMoves(side);
    for (const Move& m : legal) {
        if (m.from == from && m.to == to && m.promotion == promo) {
            out = m;
            return true;
        }
    }
    return false;
}

bool Board::isThreefoldRepetition() const {
    if (repetitionHistory.empty()) return false;
    uint64_t current = positionHash();
    int count = 0;
    for (auto h : repetitionHistory) if (h == current) ++count;
    return count >= 3;
}

void Board::pushRepetition() { repetitionHistory.push_back(positionHash()); }
void Board::popRepetition() { if (repetitionHistory.size() > 1) repetitionHistory.pop_back(); }

bool Board::isInsufficientMaterial() const {
    int bishops = 0, knights = 0, rooks = 0, queens = 0, pawns = 0;
    std::vector<int> bishopColors;
    for (int sq = 0; sq < 64; ++sq) {
        const Piece p = board[sq];
        if (p.type == PieceType::King || p.type == PieceType::None) continue;
        switch (p.type) {
            case PieceType::Bishop:
                ++bishops;
                bishopColors.push_back((rowOf(sq) + colOf(sq)) % 2);
                break;
            case PieceType::Knight: ++knights; break;
            case PieceType::Rook: ++rooks; break;
            case PieceType::Queen: ++queens; break;
            case PieceType::Pawn: ++pawns; break;
            default: break;
        }
    }
    if (pawns || rooks || queens) return false;
    if (bishops == 0 && knights == 0) return true; // K vs K
    if (bishops == 1 && knights == 0) return true; // K+B vs K
    if (bishops == 0 && knights == 1) return true; // K+N vs K
    if (bishops == 2 && knights == 0) {
        return bishopColors[0] == bishopColors[1]; // K+B vs K+B same-color bishops
    }
    return false;
}

} // namespace chess
