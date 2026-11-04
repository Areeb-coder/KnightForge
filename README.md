# MyChessEngine

A C++ chess-engine project preserved across four development milestones. The repository uses branches to keep each supplied snapshot independently accessible.

## Development branches

| Branch | Snapshot | Description |
|---|---|---|
| `rules/complete-chess-logic` | Phase 3 | Foundational board and move-generation milestone |
| `engine/basic-search` | Final | Supplied classical engine/search snapshot |
| `engine/advanced-search` | Pre-ML | Later classical-engine iteration; not a trained ML model |
| `main` | SAN | SAN-enabled snapshot and recommended default branch |

Switch branches from GitHub's branch selector to inspect each version. Each branch contains its own `BRANCH_VERSION.md` and the source snapshot for that milestone.

## Clone

```bash
git clone https://github.com/YOUR_USERNAME/MyChessEngine.git
cd MyChessEngine
git branch -a
```

To check out a particular version:

```bash
git switch rules/complete-chess-logic
git switch engine/basic-search
git switch engine/advanced-search
git switch main
```

## Build the SAN default branch

The detailed SAN engine guide is available in [`docs/SAN_ENGINE_README.md`](docs/SAN_ENGINE_README.md). See the branch-specific `README.md` for build instructions. On Windows, use `build_windows.bat` if provided. On Linux/macOS, use `./build_linux.sh` if provided, or build with CMake.

## Notes

- The branches preserve the supplied ZIP snapshots; branch order is an organizational development sequence, not a claim that every feature was independently verified.
- No Elo rating is claimed. Establish playing strength with a controlled engine tournament.
- No license is added. If you want others to reuse or redistribute the code, choose an appropriate license after confirming ownership and third-party rights.
