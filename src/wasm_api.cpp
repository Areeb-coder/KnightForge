#include "Chess.h"

#include <algorithm>
#include <cctype>
#include <string>

#include <emscripten/emscripten.h>

namespace {
bool parseUci(const std::string& text, Move& out) {
    if (text.size() != 4 && text.size() != 5) return false;
    std::string s = text;
    for (char& ch : s) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    if (s[0] < 'a' || s[0] > 'h' || s[2] < 'a' || s[2] > 'h' ||
        s[1] < '1' || s[1] > '8' || s[3] < '1' || s[3] > '8') return false;

    out.fromRow = '8' - s[1];
    out.fromCol = s[0] - 'a';
    out.toRow = '8' - s[3];
    out.toCol = s[2] - 'a';
    out.promotion = PieceType::NONE;

    if (s.size() == 5) {
        switch (s[4]) {
            case 'q': out.promotion = PieceType::QUEEN; break;
            case 'r': out.promotion = PieceType::ROOK; break;
            case 'b': out.promotion = PieceType::BISHOP; break;
            case 'n': out.promotion = PieceType::KNIGHT; break;
            default: return false;
        }
    }
    return true;
}

bool sameMove(const Move& a, const Move& b) {
    return a.fromRow == b.fromRow && a.fromCol == b.fromCol &&
           a.toRow == b.toRow && a.toCol == b.toCol &&
           a.promotion == b.promotion;
}

bool loadPosition(const char* fen, Position& position) {
    return fen && positionFromFEN(fen, position);
}
} // namespace

extern "C" {

EMSCRIPTEN_KEEPALIVE
const char* knightforge_legal_moves(const char* fen) {
    static std::string output;
    Position position;
    if (!loadPosition(fen, position)) return "ERROR invalid FEN";

    output.clear();
    const auto moves = generateLegalMoves(position);
    for (const auto& move : moves) {
        if (!output.empty()) output.push_back(' ');
        output += moveToString(move);
    }
    return output.c_str();
}

EMSCRIPTEN_KEEPALIVE
const char* knightforge_apply_move(const char* fen, const char* uciMove) {
    static std::string output;
    Position position;
    Move requested{};
    if (!loadPosition(fen, position) || !uciMove || !parseUci(uciMove, requested)) {
        return "ERROR invalid position or move";
    }

    const auto legal = generateLegalMoves(position);
    const auto found = std::find_if(legal.begin(), legal.end(),
        [&](const Move& candidate) { return sameMove(candidate, requested); });
    if (found == legal.end()) return "ERROR illegal move";

    const std::string san = moveToSAN(position, *found);
    StateInfo state{};
    makeMove(position, *found, state);
    output = san + "\n" + positionToFEN(position);
    return output.c_str();
}

EMSCRIPTEN_KEEPALIVE
const char* knightforge_best_move(const char* fen, int depth, int timeLimitMs) {
    static std::string output;
    Position position;
    if (!loadPosition(fen, position)) return "ERROR invalid FEN";

    const int safeDepth = std::max(1, std::min(depth, 10));
    const int safeTime = std::max(50, std::min(timeLimitMs, 10000));
    const SearchResult result = searchBestMove(position, safeDepth, safeTime);
    output = result.depth == 0 ? "0000" : moveToString(result.move);
    return output.c_str();
}

EMSCRIPTEN_KEEPALIVE
const char* knightforge_status(const char* fen) {
    static std::string output;
    Position position;
    if (!loadPosition(fen, position)) return "error";

    const auto legal = generateLegalMoves(position);
    if (legal.empty()) {
        output = isInCheck(position, position.sideToMove) ? "checkmate" : "stalemate";
    } else if (position.halfmoveClock >= 100) {
        output = "draw50";
    } else if (isInCheck(position, position.sideToMove)) {
        output = "check";
    } else {
        output = "playing";
    }
    return output.c_str();
}

} // extern "C"
