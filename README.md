# KnightForge

**A C++17 chess engine built around legal move generation, Standard Algebraic Notation, classical search, and the Universal Chess Interface (UCI).**

KnightForge is developed as a set of independently inspectable milestones. The current `main` branch is the SAN-enabled classical engine snapshot.

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=c%2B%2B&logoColor=white" alt="C++17" />
  <img src="https://img.shields.io/badge/Protocol-UCI-6D5EF5?style=flat-square" alt="UCI protocol" />
  <img src="https://img.shields.io/badge/Notation-SAN-0E9F6E?style=flat-square" alt="SAN notation" />
  <img src="https://img.shields.io/badge/Build-CMake-064F8C?style=flat-square&logo=cmake&logoColor=white" alt="CMake" />
</p>

---

## What it can do

- Generate and validate legal moves.
- Parse and produce Standard Algebraic Notation (SAN).
- Detect check, checkmate, and stalemate.
- Handle castling, en passant, and promotion choices.
- Load and export FEN positions.
- Make and undo moves.
- Track the 50-move rule and threefold repetition in the terminal game loop.
- Search with negamax, alpha-beta pruning, iterative deepening, quiescence search, move ordering, killer/history heuristics, and a transposition table.
- Evaluate positions using material, piece-square tables, pawn structure, passed pawns, bishop pair, rook files, and king-safety heuristics.
- Communicate with compatible chess GUIs through UCI.
- Run perft and SAN regression tests.

> **Playing-strength status:** no verified Elo is published. Search depth, engine features, and perft correctness are not substitutes for a color-balanced match tournament against reference engines.

## Architecture

```text
KnightForge/
├── .github/workflows/    # CI build and test workflow
├── docs/                 # SAN, UCI, and testing documentation
├── src/
│   ├── Phase1/           # Piece and square foundations
│   ├── Phase2/           # Move representation
│   ├── Chess.cpp         # Rules, move generation, evaluation, search
│   ├── Chess.h
│   ├── UCI.cpp           # UCI protocol loop
│   ├── UCI.h
│   └── main.cpp          # Terminal interface
├── tests/                # Perft and SAN regression tests
├── CMakeLists.txt
├── build_linux.sh
└── build_windows.bat
```

## Build

### Requirements

- CMake 3.16+
- A C++17 compiler (GCC, Clang, Visual Studio, or MinGW)

### Windows (Visual Studio)

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Run the engine:

```powershell
.\build\Release\chess_engine.exe
```

### Linux / macOS

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
./build/chess_engine
```

## Play in the terminal

The terminal interface accepts SAN moves:

```text
e4       play a pawn move
Nf3      play a knight move
O-O      castle kingside
moves    list legal moves
fen      print the current FEN
undo     undo the previous move
new      start a new game
depth 5  set search depth
time 1000 set search time in milliseconds
bench    run a search benchmark
help     list commands
quit     exit
```

The terminal loop currently has the engine respond after the human move. For a conventional human-vs-engine game, start from the default position and play White.

## Connect a chess GUI (UCI)

Launch:

```bash
./build/chess_engine --uci
```

UCI uses coordinate notation. A minimal protocol sequence is:

```text
uci
isready
position startpos moves e2e4 e7e5 g1f3
go depth 5
```

The engine responds with search information and a `bestmove`. Configure the executable as a UCI engine in a compatible desktop chess GUI.

## Development milestones

| Branch | Milestone |
|---|---|
| `rules/complete-chess-logic` | Foundational board and move-generation work |
| `engine/basic-search` | Classical engine/search snapshot |
| `engine/advanced-search` | Later classical-engine iteration |
| `main` | SAN-enabled engine snapshot |

The branch names preserve the supplied development snapshots; they do not imply that every feature has been independently benchmarked.

## Tests

```bash
ctest --test-dir build --output-on-failure
```

The perft test checks the standard initial-position node counts:

| Depth | Expected nodes |
|---:|---:|
| 1 | 20 |
| 2 | 400 |
| 3 | 8,902 |
| 4 | 197,281 |

These tests cover selected behavior, not every possible chess position. See [TESTING.md](docs/TESTING.md) and [SAN_ENGINE_README.md](docs/SAN_ENGINE_README.md) for additional details.

## Play in your browser

A browser chess player is included in [`web/`](web/). It compiles the repository's C++ move-generation and search implementation to WebAssembly, so the opponent is KnightForge itself—not a remote third-party engine.

- You play White; KnightForge plays Black.
- Legal moves, SAN notation, and engine search run locally in the browser.
- Search depth and per-move thinking time are configurable.
- The page includes move history, undo, new game, and FEN copying.

**Publishing:** the GitHub Actions workflow at [`.github/workflows/pages.yml`](.github/workflows/pages.yml) builds the WebAssembly module and deploys `web/` to GitHub Pages. In the repository, open **Settings → Pages → Build and deployment → Source**, select **GitHub Actions**, and let the workflow complete. The expected URL is `https://areeb-coder.github.io/KnightForge/`.

See [`docs/WEB_PLAYER.md`](docs/WEB_PLAYER.md) for the architecture, setup, and current limitations.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). Please include reproducible steps and test output for correctness or search changes.

## License

No open-source license is currently included. Do not assume this repository grants redistribution or modification rights until a license is added by the project owner.
