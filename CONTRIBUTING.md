# Contributing

Thanks for considering a contribution to MyChessEngine SAN.

## Before opening a pull request

1. Create a focused branch for your change.
2. Build with CMake and a C++17 compiler.
3. Run the test suite with `ctest --test-dir build --output-on-failure`.
4. Add or update regression tests for rule, notation, or search changes.
5. Describe the behavior changed and include reproducible test output.

## Chess correctness changes

- Preserve legal-move validation, including king safety.
- Test special moves: castling, en passant, and promotion.
- Test check, checkmate, stalemate, and draw conditions where relevant.
- Avoid presenting a search-strength improvement without paired benchmark evidence.

## Style

Keep changes small and consistent with the surrounding C++ code. Prefer clear names and comments that explain non-obvious chess or search logic.
