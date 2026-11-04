#include "Chess.h"
#include "UCI.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

bool sameMove(const Move& a, const Move& b) {
    return a.fromRow == b.fromRow && a.fromCol == b.fromCol &&
           a.toRow == b.toRow && a.toCol == b.toCol &&
           a.promotion == b.promotion;
}

bool parseUciMove(const std::string& text, Move& out) {
    if (text.size() != 4 && text.size() != 5) return false;
    std::string s = text;
    for (char& ch : s) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    if (s[0] < 'a' || s[0] > 'h' || s[2] < 'a' || s[2] > 'h' ||
        s[1] < '1' || s[1] > '8' || s[3] < '1' || s[3] > '8') return false;
    out.fromRow = '8' - s[1]; out.fromCol = s[0] - 'a';
    out.toRow = '8' - s[3]; out.toCol = s[2] - 'a';
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

void applyUciMoves(Position& p, const std::vector<std::string>& tokens) {
    for (const auto& token : tokens) {
        Move requested{};
        if (!parseUciMove(token, requested)) continue;
        auto legal = generateLegalMoves(p);
        auto it = std::find_if(legal.begin(), legal.end(), [&](const Move& m) { return sameMove(m, requested); });
        if (it == legal.end()) continue;
        StateInfo st{};
        makeMove(p, *it, st);
    }
}

} // namespace

void runUCI() {
    Position p = startPosition();
    int defaultDepth = 5;
    int defaultMoveTime = 1000;

    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream in(line);
        std::string cmd;
        in >> cmd;

        if (cmd == "uci") {
            std::cout << "id name MyChessEngine Pre-ML\n";
            std::cout << "id author Areeb\n";
            std::cout << "option name Hash type spin default 64 min 1 max 1024\n";
            std::cout << "option name Depth type spin default 5 min 1 max 12\n";
            std::cout << "uciok\n";
        } else if (cmd == "isready") {
            std::cout << "readyok\n";
        } else if (cmd == "ucinewgame") {
            p = startPosition();
        } else if (cmd == "position") {
            std::string what;
            in >> what;
            if (what == "startpos") {
                p = startPosition();
                std::string maybeMoves;
                in >> maybeMoves;
                if (maybeMoves == "moves") {
                    std::vector<std::string> tokens;
                    std::string t;
                    while (in >> t) tokens.push_back(t);
                    applyUciMoves(p, tokens);
                }
            } else if (what == "fen") {
                std::string fen, part;
                std::getline(in, part);
                std::istringstream fenStream(part);
                std::vector<std::string> fields;
                std::string f;
                while (fenStream >> f) fields.push_back(f);
                if (fields.size() >= 4) {
                    fen.clear();
                    for (size_t i = 0; i < fields.size() && i < 6; ++i) {
                        if (i) fen += ' ';
                        fen += fields[i];
                    }
                    Position q;
                    if (positionFromFEN(fen, q)) p = q;
                    auto pos = std::find(fields.begin(), fields.end(), "moves");
                    if (pos != fields.end()) {
                        std::vector<std::string> tokens(pos + 1, fields.end());
                        applyUciMoves(p, tokens);
                    }
                }
            }
        } else if (cmd == "go") {
            int depth = defaultDepth;
            int timeMs = defaultMoveTime;
            std::string t;
            while (in >> t) {
                if (t == "depth") in >> depth;
                else if (t == "movetime") in >> timeMs;
                else if (t == "wtime" || t == "btime") {
                    int ignored = 0; in >> ignored;
                } else if (t == "infinite") {
                    timeMs = 60000;
                }
            }
            SearchResult r = searchBestMove(p, depth, timeMs);
            std::cout << "info depth " << r.depth
                      << " score cp " << r.score
                      << " nodes " << r.nodes << '\n';
            std::cout << "bestmove " << moveToString(r.move) << '\n';
        } else if (cmd == "setoption") {
            // Options are accepted for compatibility. The current engine keeps fixed TT size.
        } else if (cmd == "quit") {
            break;
        }
        std::cout.flush();
    }
}
