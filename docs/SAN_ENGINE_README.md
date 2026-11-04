# MyChessEngine SAN

A C++17 chess engine with a terminal interface, Standard Algebraic Notation (SAN), legal move generation, classical search, and a Universal Chess Interface (UCI) mode.

> **Status:** Classical, pre-machine-learning engine. No Elo rating is claimed: playing strength has not yet been established through a statistically meaningful match tournament.

## Highlights

- Legal chess move generation and validation
- Standard Algebraic Notation (SAN)
- Check, checkmate, and stalemate detection
- Castling, en passant, and all promotion choices
- FEN position loading and export
- Make/undo move support
- 50-move and threefold-repetition tracking
- Insufficient-material draw detection
- Classical search with negamax, alpha-beta pruning, move ordering, quiescence search, and iterative deepening
- Evaluation features including material, piece-square tables, pawn structure, passed pawns, bishop pair, rook files, and king safety
- Killer-move and history heuristics
- Transposition table
- UCI mode for compatible chess GUIs
- Perft and SAN regression tests

## Repository structure

```text
MyChessEngine_SAN/
├── .github/workflows/       # Automated build and test workflow
├── docs/                    # Project and protocol notes
├── src/
│   ├── Phase1/              # Piece and square foundations
│   ├── Phase2/              # Move representation and helpers
│   ├── Chess.cpp            # Position, rules, move generation, search
│   ├── Chess.h
│   ├── UCI.cpp              # UCI protocol loop
│   ├── UCI.h
│   └── main.cpp             # Terminal entry point
├── tests/
│   ├── perft_test.cpp
│   └── san_test.cpp
├── CMakeLists.txt
├── build_linux.sh
├── build_windows.bat
└── README.md
```

## Requirements

- CMake 3.16 or newer
- A C++17 compiler
  - Windows: Visual Studio 2019/2022 C++ workload or MinGW-w64
  - Linux: GCC or Clang
  - macOS: Apple Clang with Xcode Command Line Tools

## Build

### Windows — Visual Studio compiler

Run in PowerShell or Command Prompt from the repository root:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

The executable is typically `build\Release\chess_engine.exe` (or `build\chess_engine.exe` with a single-configuration generator).

### Windows — MinGW / MSYS2

```bash
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Linux / macOS

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The executable is `build/chess_engine`.

## Run the engine

Start the interactive terminal interface:

```bash
# Linux / macOS
./build/chess_engine

# Windows PowerShell (adjust path for your generator)
.\build\Release\chess_engine.exe
```

The terminal interface accepts SAN moves and provides commands such as:

| Command | Purpose |
|---|---|
| `e4`, `Nf3`, `Bxc6`, `O-O` | Play a legal SAN move |
| `moves` | List legal moves |
| `fen` | Print the current FEN |
| `setfen <FEN>` | Load a position |
| `undo` | Undo the previous move |
| `new` | Start a new game |
| `depth N` | Set the search depth limit |
| `time N` | Set the search time in milliseconds |
| `perft N` | Run a move-generation count |
| `divide N` | Show per-root perft counts |
| `bench` | Run a search benchmark |
| `uci` | Enter UCI mode |
| `help` | Show available commands |
| `quit` | Exit |

## Use with a chess GUI (UCI)

Start the engine directly in UCI mode:

```bash
./build/chess_engine --uci
```

Then configure the executable as a UCI engine in a compatible chess GUI. For a basic protocol check, send:

```text
uci
isready
position startpos moves e2e4 e7e5 g1f3
 go depth 5
```

The engine should answer the handshake and eventually return a `bestmove` for a valid search request. UCI uses coordinate notation (for example `e2e4`), while the interactive terminal accepts SAN (for example `e4`).

## Run tests

```bash
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

The included perft test checks standard starting-position node counts:

| Depth | Expected nodes |
|---:|---:|
| 1 | 20 |
| 2 | 400 |
| 3 | 8,902 |
| 4 | 197,281 |

The SAN test exercises notation handling. These tests validate selected correctness properties; they do not prove complete chess-rule correctness or establish playing strength.

## Current scope and roadmap

This repository contains the classical engine foundation and a WebAssembly browser player. It does **not** currently include neural-network evaluation, PyTorch training, reinforcement learning, self-play training, an opening book, or endgame tablebases.

Potential next steps:

1. Expand rule and edge-case regression tests.
2. Add reproducible benchmark positions and search-performance reporting.
3. Measure playing strength against fixed reference engines with paired games as both colors.
4. Add a trained evaluation model only as a separately benchmarked future milestone.

## Playing-strength disclaimer

No verified Elo is published for this engine at this time. A reliable estimate requires games against reference engines with known ratings, controlled time controls, color-balanced pairings, and enough games to report uncertainty. Search depth or passing perft tests alone cannot be converted into an Elo rating.

## Contributing

Issues and pull requests are welcome. Please include the compiler/platform, build command, exact reproduction steps, and test output when reporting a bug. See [CONTRIBUTING.md](CONTRIBUTING.md).

## License

No open-source license has been added yet. Unless the project owner adds a license, normal copyright applies and others do not automatically receive permission to reuse, modify, or redistribute the code. Add a license only after deciding which permissions you want to grant.
