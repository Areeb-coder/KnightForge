# KnightForge browser player

The page in this directory runs the KnightForge C++ engine in a browser using WebAssembly.

## How it works

- `src/wasm_api.cpp` exposes a small C ABI for legal move generation, move application/SAN, engine search, and game status.
- GitHub Actions compiles the existing `Chess.cpp`, `Piece.cpp`, and `Move.cpp` sources with Emscripten.
- `web/app.js` loads the generated WebAssembly module and uses it for both legal moves and the engine's response.
- The board, controls, and move history are static HTML/CSS/JavaScript. No chess engine is fetched from a third-party service.

## Publish with GitHub Pages

1. Open the repository's **Settings → Pages**.
2. Under **Build and deployment**, set **Source** to **GitHub Actions**.
3. Push to `main` or manually run **Build and deploy KnightForge web player** under Actions.
4. When the workflow succeeds, open: https://areeb-coder.github.io/KnightForge/

The workflow is defined at [`.github/workflows/pages.yml`](../.github/workflows/pages.yml).

## Build locally

With Emscripten installed and activated:

```bash
mkdir -p web/engine
em++ src/Chess.cpp src/Phase1/Piece.cpp src/Phase2/Move.cpp src/wasm_api.cpp \
  -O3 -std=c++17 --no-entry \
  -s WASM=1 \
  -s MODULARIZE=1 \
  -s EXPORT_ES6=1 \
  -s EXPORT_NAME=createKnightForge \
  -s ENVIRONMENT=web \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s NO_EXIT_RUNTIME=1 \
  -s EXPORTED_FUNCTIONS='["_knightforge_legal_moves","_knightforge_apply_move","_knightforge_best_move","_knightforge_status"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall"]' \
  -o web/engine/knightforge.js
```

Serve the `web/` directory over HTTP(S); opening `index.html` as a local file may be blocked by browser module/WASM security rules.

## Current limitations

- This is an experimental player, not a rated online service.
- The web API evaluates the current FEN; it does not carry a full repetition history, so threefold-repetition adjudication is not yet implemented in the browser player.
- The page does not implement clocks, online multiplayer, accounts, or cloud game storage.
- No Elo is claimed. Establish playing strength through color-balanced testing against reference engines.
- The project currently has no open-source license; see the root README before redistributing it.
