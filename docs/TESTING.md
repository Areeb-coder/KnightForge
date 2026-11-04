# Testing and benchmark notes

## Correctness tests

Configure and build with CMake, then run:

```bash
ctest --test-dir build --output-on-failure
```

The current perft test checks the standard initial-position node counts through depth four. The SAN test covers the notation cases encoded in `tests/san_test.cpp`.

## UCI smoke test

Launch `chess_engine --uci` and send `uci`, `isready`, `position startpos`, and `go depth 3`. Confirm that the engine emits `uciok`, `readyok`, and a `bestmove` response.

## Playing-strength measurement

Do not infer Elo from search depth, a single game, or perft. For a defensible estimate:

- Select reference engines with known ratings and compatible versions.
- Fix the time control, hardware, and engine options.
- Play paired games with each engine playing both White and Black.
- Use opening positions or a documented opening suite to reduce starting-position bias.
- Record wins, draws, losses, game count, and a confidence interval.
- Keep the engine binary and configuration identified for reproducibility.
