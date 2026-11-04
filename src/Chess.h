#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <random>
#include <unordered_map>
#include <algorithm>
#include <limits>
#include <cctype>

namespace chess {

enum class Color : uint8_t { None = 0, White = 1, Black = 2 };

enum class PieceType : uint8_t {
    None = 0, Pawn, Knight, Bishop, Rook, Queen, King
};

struct Piece {
    PieceType type{PieceType::None};
    Color color{Color::None};
};

struct Move {
    int from{-1};
    int to{-1};
    PieceType promotion{PieceType::None};
    bool isEnPassant{false};
    bool isCastle{false};

    bool operator==(const Move& other) const {
        return from == other.from && to == other.to &&
               promotion == other.promotion &&
               isEnPassant == other.isEnPassant &&
               isCastle == other.isCastle;
    }
};

inline Color opposite(Color c) {
    if (c == Color::White) return Color::Black;
    if (c == Color::Black) return Color::White;
    return Color::None;
}

inline int rowOf(int sq) { return sq / 8; }
inline int colOf(int sq) { return sq % 8; }
inline bool onBoard(int r, int c) { return r >= 0 && r < 8 && c >= 0 && c < 8; }
inline int makeSquare(int r, int c) { return r * 8 + c; }

inline char pieceToChar(Piece p) {
    char ch = '.';
    switch (p.type) {
        case PieceType::Pawn: ch = 'p'; break;
        case PieceType::Knight: ch = 'n'; break;
        case PieceType::Bishop: ch = 'b'; break;
        case PieceType::Rook: ch = 'r'; break;
        case PieceType::Queen: ch = 'q'; break;
        case PieceType::King: ch = 'k'; break;
        default: return '.';
    }
    if (p.color == Color::White) ch = static_cast<char>(std::toupper(ch));
    return ch;
}

inline Piece charToPiece(char ch) {
    if (ch == '.') return {};
    Color color = std::isupper(static_cast<unsigned char>(ch)) ? Color::White : Color::Black;
    char x = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    PieceType type = PieceType::None;
    switch (x) {
        case 'p': type = PieceType::Pawn; break;
        case 'n': type = PieceType::Knight; break;
        case 'b': type = PieceType::Bishop; break;
        case 'r': type = PieceType::Rook; break;
        case 'q': type = PieceType::Queen; break;
        case 'k': type = PieceType::King; break;
        default: return {};
    }
    return {type, color};
}

inline std::string squareToString(int sq) {
    if (sq < 0 || sq >= 64) return "??";
    std::string s;
    s += static_cast<char>('a' + colOf(sq));
    s += static_cast<char>('1' + rowOf(sq));
    return s;
}

inline int stringToSquare(const std::string& s) {
    if (s.size() != 2) return -1;
    char file = static_cast<char>(std::tolower(static_cast<unsigned char>(s[0])));
    char rank = s[1];
    if (file < 'a' || file > 'h' || rank < '1' || rank > '8') return -1;
    return makeSquare(rank - '1', file - 'a');
}

inline char promotionToChar(PieceType p) {
    switch (p) {
        case PieceType::Queen: return 'q';
        case PieceType::Rook: return 'r';
        case PieceType::Bishop: return 'b';
        case PieceType::Knight: return 'n';
        default: return '\0';
    }
}

inline std::string moveToUci(const Move& m) {
    std::string s = squareToString(m.from) + squareToString(m.to);
    if (m.promotion != PieceType::None) s += promotionToChar(m.promotion);
    return s;
}

struct UndoState {
    Piece moved{};
    Piece captured{};
    int capturedSquare{-1};
    int previousEnPassant{-1};
    bool wK{true}, wQ{true}, bK{true}, bQ{true};
    int halfmove{0};
    int fullmove{1};
    uint64_t hash{0};
};

class Board {
public:
    Board();
    void reset();
    void print(std::ostream& out = std::cout) const;

    bool fromFEN(const std::string& fen, std::string* error = nullptr);
    std::string toFEN() const;

    std::vector<Move> generatePseudoLegalMoves(Color side) const;
    std::vector<Move> generateLegalMoves(Color side);

    bool isSquareAttacked(int sq, Color by) const;
    bool isInCheck(Color side) const;
    bool makeMove(const Move& move, UndoState& undo);
    void undoMove(const Move& move, const UndoState& undo);

    bool parseUserMove(const std::string& input, Move& out) const;
    bool isThreefoldRepetition() const;
    bool isInsufficientMaterial() const;
    bool isFiftyMoveDraw() const { return halfmoveClock >= 100; }

    Color sideToMove() const { return side; }
    int getEnPassantSquare() const { return enPassantSquare; }
    int getHalfmoveClock() const { return halfmoveClock; }
    int getFullmoveNumber() const { return fullmoveNumber; }
    bool canCastleKingSide(Color c) const { return c == Color::White ? whiteKingCastle : blackKingCastle; }
    bool canCastleQueenSide(Color c) const { return c == Color::White ? whiteQueenCastle : blackQueenCastle; }
    const std::array<Piece,64>& squares() const { return board; }
    uint64_t hash() const { return positionHash(); }

    int findKing(Color c) const;

    void pushRepetition();
    void popRepetition();

private:
    std::array<Piece,64> board{};
    Color side{Color::White};
    bool whiteKingCastle{true}, whiteQueenCastle{true};
    bool blackKingCastle{true}, blackQueenCastle{true};
    int enPassantSquare{-1};
    int halfmoveClock{0};
    int fullmoveNumber{1};
    std::vector<uint64_t> repetitionHistory;

    uint64_t positionHash() const;
    void setError(std::string* error, const std::string& msg) const;
    void updateCastlingRightsForMove(const Move& move, const UndoState& undo);
};

class MoveGenerator {
public:
    static void addPawnMoves(const Board& b, Color side, int sq, std::vector<Move>& out);
    static void addKnightMoves(const Board& b, Color side, int sq, std::vector<Move>& out);
    static void addSlidingMoves(const Board& b, Color side, int sq, const std::vector<std::pair<int,int>>& dirs, std::vector<Move>& out);
    static void addKingMoves(const Board& b, Color side, int sq, std::vector<Move>& out);
};

class Search {
public:
    Move findBestMove(Board& board, int depth);
    int evaluate(const Board& board) const;

private:
    struct TTEntry { int depth; int score; uint8_t flag; Move best; };
    std::unordered_map<uint64_t, TTEntry> tt;
    uint64_t nodes{0};

    int alphaBeta(Board& board, int depth, int alpha, int beta, int ply);
    int quiescence(Board& board, int alpha, int beta, int ply);
    int pieceValue(PieceType t) const;
    int moveScore(const Board& board, const Move& move) const;
    static constexpr int INF = 1000000000;
    static constexpr int MATE = 1000000;
};

} // namespace chess
