#include "Chess.h"

using namespace chess;

namespace {

void printHelp() {
    std::cout
        << "Commands:\n"
        << "  e2e4          Make a move\n"
        << "  e7e8q         Promote pawn to queen (q/r/b/n)\n"
        << "  moves         Show all legal moves\n"
        << "  fen            Show current FEN\n"
        << "  setfen <FEN>  Load a FEN position\n"
        << "  undo           Undo the previous move\n"
        << "  new            Reset to starting position\n"
        << "  help           Show commands\n"
        << "  quit           Exit\n\n"
        << "Phase 3 is rules-only: White and Black play from the terminal.\n";
}

void printStatus(Board& board) {
    Color side = board.sideToMove();
    auto legal = board.generateLegalMoves(side);

    std::cout << (side == Color::White ? "White" : "Black") << " to move";
    if (board.isInCheck(side)) std::cout << " — CHECK";
    std::cout << "\n";

    if (legal.empty()) {
        if (board.isInCheck(side)) {
            std::cout << "CHECKMATE! " << (side == Color::White ? "Black" : "White") << " wins.\n";
        } else {
            std::cout << "STALEMATE! Draw.\n";
        }
        return;
    }

    if (board.isThreefoldRepetition()) {
        std::cout << "Draw available/claimed by threefold repetition.\n";
    }
    if (board.isFiftyMoveDraw()) {
        std::cout << "50-move draw condition reached.\n";
    }
    if (board.isInsufficientMaterial()) {
        std::cout << "Draw by insufficient material.\n";
    }
}

bool isTerminal(Board& board) {
    Color side = board.sideToMove();
    auto legal = board.generateLegalMoves(side);
    if (legal.empty()) {
        printStatus(board);
        return true;
    }
    if (board.isFiftyMoveDraw() || board.isThreefoldRepetition() || board.isInsufficientMaterial()) {
        printStatus(board);
        return true;
    }
    return false;
}

} // namespace

int main() {
    Board board;
    std::vector<Board> history;
    history.push_back(board);

    std::cout << "========================================\n"
              << "          CHESS ENGINE — PHASE 3       \n"
              << "========================================\n"
              << "Complete chess rules, terminal only.\n"
              << "White and Black play manually.\n"
              << "Type 'help' for commands.\n";

    while (true) {
        board.print();
        printStatus(board);

        if (isTerminal(board)) break;

        std::cout << "> ";
        std::string input;
        if (!std::getline(std::cin, input)) break;
        if (input.empty()) continue;

        std::istringstream iss(input);
        std::string cmd;
        iss >> cmd;
        std::string lower = cmd;
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (lower == "quit" || lower == "exit") break;
        if (lower == "help") { printHelp(); continue; }

        if (lower == "new") {
            board.reset();
            history.clear();
            history.push_back(board);
            continue;
        }

        if (lower == "fen") {
            std::cout << board.toFEN() << "\n";
            continue;
        }

        if (lower == "setfen") {
            std::string fen;
            std::getline(iss >> std::ws, fen);
            std::string error;
            if (board.fromFEN(fen, &error)) {
                history.clear();
                history.push_back(board);
                std::cout << "FEN loaded.\n";
            } else {
                std::cout << "FEN error: " << error << "\n";
            }
            continue;
        }

        if (lower == "moves") {
            auto legal = board.generateLegalMoves(board.sideToMove());
            for (const auto& move : legal) std::cout << moveToUci(move) << ' ';
            std::cout << "\nTotal: " << legal.size() << "\n";
            continue;
        }

        if (lower == "undo") {
            if (history.size() > 1) {
                history.pop_back();
                board = history.back();
                std::cout << "Move undone.\n";
            } else {
                std::cout << "Nothing to undo.\n";
            }
            continue;
        }

        Move move;
        if (!board.parseUserMove(input, move)) {
            std::cout << "Illegal move. Use coordinate notation such as e2e4 or e7e8q.\n";
            continue;
        }

        UndoState undo;
        if (!board.makeMove(move, undo)) {
            std::cout << "Could not make move.\n";
            continue;
        }
        board.pushRepetition();
        history.push_back(board);
    }

    std::cout << "Goodbye.\n";
    return 0;
}
