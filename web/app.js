const START_FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1';
const SYMBOLS = {
  K: '♔', Q: '♕', R: '♖', B: '♗', N: '♘', P: '♙',
  k: '♚', q: '♛', r: '♜', b: '♝', n: '♞', p: '♟'
};

const boardEl = document.querySelector('#board');
const statusEl = document.querySelector('#status');
const engineStateEl = document.querySelector('#engine-state');
const statusDot = document.querySelector('#status-dot');
const movesEl = document.querySelector('#moves');
const moveCountEl = document.querySelector('#move-count');
const fenEl = document.querySelector('#fen');
const depthEl = document.querySelector('#depth');
const depthValueEl = document.querySelector('#depth-value');
const thinkTimeEl = document.querySelector('#think-time');
const undoButton = document.querySelector('#undo');
const sideModal = document.querySelector('#side-modal');
const modalEngineStatus = document.querySelector('#modal-engine-status');
const promotionModal = document.querySelector('#promotion-modal');

let engine = null;
let fen = START_FEN;
let selected = null;
let legalMoves = [];
let moveLog = [];
let turnSnapshots = [];
let busy = true;
let finished = false;
let lastMove = null;
let playerColor = 'w';
let gameStarted = false;
let promotionResolver = null;

function setStatus(message, ready = true) {
  statusEl.textContent = message;
  statusDot.classList.toggle('ready', ready);
}

function boardFromFen(position) {
  const rows = position.split(' ')[0].split('/');
  const squares = [];
  for (const row of rows) {
    for (const token of row) {
      if (/^[1-8]$/.test(token)) {
        for (let i = 0; i < Number(token); i++) squares.push(null);
      } else {
        squares.push(token);
      }
    }
  }
  return squares;
}

function pieceColor(piece) {
  return piece && piece === piece.toUpperCase() ? 'w' : 'b';
}

function isPlayerPiece(piece) {
  return Boolean(piece && pieceColor(piece) === playerColor);
}

function squareName(row, col) {
  return String.fromCharCode(97 + col) + String(8 - row);
}

function renderBoard() {
  const squares = boardFromFen(fen);
  const targets = selected ? legalMoves.filter(move => move.slice(0, 2) === selected).map(move => move.slice(2, 4)) : [];
  const playerIsWhite = playerColor === 'w';
  boardEl.innerHTML = '';

  for (let displayIndex = 0; displayIndex < 64; displayIndex++) {
    const displayRow = Math.floor(displayIndex / 8);
    const displayCol = displayIndex % 8;
    const row = playerIsWhite ? displayRow : 7 - displayRow;
    const col = playerIsWhite ? displayCol : 7 - displayCol;
    const name = squareName(row, col);
    const piece = squares[row * 8 + col];
    const button = document.createElement('button');
    button.type = 'button';
    button.className = 'square ' + ((displayRow + displayCol) % 2 === 0 ? 'light' : 'dark');
    button.dataset.square = name;
    button.setAttribute('aria-label', name + (piece ? ' ' + piece : ' empty'));

    if (name === selected) button.classList.add('selected');
    if (lastMove && (name === lastMove.slice(0, 2) || name === lastMove.slice(2, 4))) button.classList.add('last-move');
    if (targets.includes(name)) {
      button.classList.add('legal');
      if (piece) button.classList.add('capture');
    }

    if (piece) {
      const pieceEl = document.createElement('span');
      pieceEl.className = 'piece ' + (pieceColor(piece) === 'w' ? 'white' : 'black');
      pieceEl.textContent = SYMBOLS[piece];
      button.append(pieceEl);
    }

    if (displayCol === 0) {
      const coord = document.createElement('span');
      coord.className = 'coord';
      coord.textContent = String(8 - row);
      button.append(coord);
    }
    if (displayRow === 7) {
      const coord = document.createElement('span');
      coord.className = 'coord';
      coord.textContent = String.fromCharCode(97 + col);
      button.append(coord);
    }
    boardEl.append(button);
  }
}

function updateSideLabels() {
  const humanColor = playerColor === 'w' ? 'WHITE' : 'BLACK';
  const engineColor = playerColor === 'w' ? 'BLACK' : 'WHITE';
  document.querySelector('#player-side').textContent = humanColor;
  document.querySelector('#opponent-side').textContent = engineColor;
  document.querySelector('#player-turn-label').textContent = 'You · ' + (playerColor === 'w' ? 'White' : 'Black');
  document.querySelector('#opponent-name').textContent = 'KnightForge';
  document.querySelector('#player-name').textContent = 'You';
  document.querySelector('.player-row.you .avatar').textContent = playerColor === 'w' ? '♔' : '♚';
  document.querySelector('.player-row.opponent .avatar').textContent = playerColor === 'w' ? '♚' : '♔';
  boardEl.setAttribute('aria-label', 'Chessboard. You play ' + (playerColor === 'w' ? 'White.' : 'Black.'));
}

function renderMoves() {
  movesEl.innerHTML = '';
  if (moveLog.length === 0) {
    const empty = document.createElement('p');
    empty.className = 'empty';
    empty.textContent = 'Your game will appear here.';
    movesEl.append(empty);
  }
  for (let i = 0; i < moveLog.length; i += 2) {
    const pair = document.createElement('div');
    pair.className = 'move-pair';
    const number = document.createElement('span');
    number.className = 'move-number';
    number.textContent = (Math.floor(i / 2) + 1) + '.';
    const white = document.createElement('span');
    white.className = 'move-white';
    white.textContent = moveLog[i] || '';
    const black = document.createElement('span');
    black.className = 'move-black';
    black.textContent = moveLog[i + 1] || '';
    pair.append(number, white, black);
    movesEl.append(pair);
  }
  moveCountEl.textContent = moveLog.length + ' plies';
  undoButton.disabled = busy || !gameStarted || turnSnapshots.length === 0;
}

function refreshGame() {
  legalMoves = engine ? getLegalMoves(fen) : [];
  renderBoard();
  renderMoves();
  fenEl.textContent = fen;
}

function getLegalMoves(position) {
  const result = engine.ccall('knightforge_legal_moves', 'string', ['string'], [position]);
  if (result.startsWith('ERROR')) throw new Error(result);
  return result.trim() ? result.trim().split(/\s+/) : [];
}

function applyMove(position, move) {
  const result = engine.ccall('knightforge_apply_move', 'string', ['string', 'string'], [position, move]);
  if (result.startsWith('ERROR')) throw new Error(result);
  const parts = result.split('\n');
  return { san: parts[0], fen: parts[1] };
}

function getStatus(position) {
  return engine.ccall('knightforge_status', 'string', ['string'], [position]);
}

function finishIfNeeded() {
  const state = getStatus(fen);
  const sideToMove = fen.split(' ')[1];
  if (state === 'checkmate') {
    finished = true;
    setStatus(sideToMove === playerColor ? 'Checkmate — KnightForge wins.' : 'Checkmate — you win!');
    return true;
  }
  if (state === 'stalemate') {
    finished = true;
    setStatus('Stalemate — the game is a draw.');
    return true;
  }
  if (state === 'draw50') {
    finished = true;
    setStatus('Draw by the 50-move rule.');
    return true;
  }
  if (state === 'check') {
    setStatus(sideToMove === playerColor ? 'Your king is in check.' : 'KnightForge is in check.');
  } else {
    setStatus(sideToMove === playerColor ? 'Your move — play ' + (playerColor === 'w' ? 'White.' : 'Black.') : 'KnightForge is thinking…', !busy);
  }
  return false;
}

function requestPromotion(candidates) {
  if (candidates.length <= 1) return Promise.resolve(candidates[0] || null);
  promotionModal.classList.remove('hidden');
  return new Promise(resolve => { promotionResolver = resolve; });
}

function closePromotion(choice) {
  if (promotionResolver) promotionResolver(choice);
  promotionResolver = null;
  promotionModal.classList.add('hidden');
}

async function chooseMoveForTarget(from, to) {
  const candidates = legalMoves.filter(move => move.slice(0, 2) === from && move.slice(2, 4) === to);
  if (!candidates.length) return null;
  if (candidates.length === 1) return candidates[0];
  return requestPromotion(candidates);
}

async function playEngineMove() {
  if (!engine || finished) return;
  busy = true;
  engineStateEl.textContent = 'Calculating…';
  setStatus('KnightForge is calculating its move.', false);
  renderMoves();
  try {
    const depth = Number(depthEl.value);
    const time = Number(thinkTimeEl.value);
    const best = engine.ccall('knightforge_best_move', 'string', ['string', 'number', 'number'], [fen, depth, time]);
    if (!best || best === '0000' || best.startsWith('ERROR')) {
      finished = true;
      busy = false;
      setStatus('KnightForge could not find a move. Start a new game to continue.', false);
      renderMoves();
      return;
    }
    const response = applyMove(fen, best);
    fen = response.fen;
    moveLog.push(response.san);
    lastMove = best;
    busy = false;
    engineStateEl.textContent = 'Ready to play';
    refreshGame();
    finishIfNeeded();
    renderMoves();
  } catch (error) {
    busy = false;
    setStatus('Game error: ' + error.message, false);
    console.error(error);
  }
}

async function playHumanMove(move) {
  if (busy || finished || !engine || !gameStarted) return;
  const snapshot = { fen: fen, moveLog: moveLog.slice(), lastMove: lastMove };
  try {
    const applied = applyMove(fen, move);
    turnSnapshots.push(snapshot);
    fen = applied.fen;
    moveLog.push(applied.san);
    lastMove = move;
    selected = null;
    refreshGame();

    if (finishIfNeeded()) {
      renderMoves();
      return;
    }
    await playEngineMove();
  } catch (error) {
    busy = false;
    setStatus('Game error: ' + error.message, false);
    console.error(error);
  }
}

function startGame(side) {
  if (!engine) return;
  playerColor = side;
  gameStarted = true;
  fen = START_FEN;
  selected = null;
  moveLog = [];
  turnSnapshots = [];
  lastMove = null;
  finished = false;
  busy = false;
  sideModal.classList.add('hidden');
  updateSideLabels();
  refreshGame();
  engineStateEl.textContent = 'Ready to play';
  setStatus('New game. You play ' + (playerColor === 'w' ? 'White.' : 'Black.'));
  if (playerColor === 'b') void playEngineMove();
}

function openSideModal() {
  busy = true;
  sideModal.classList.remove('hidden');
  modalEngineStatus.textContent = engine ? 'Engine ready — choose a color.' : 'Loading your engine…';
  document.querySelectorAll('.side-choice').forEach(button => { button.disabled = !engine; });
  renderMoves();
}

function handleSquareClick(event) {
  const button = event.target.closest('button[data-square]');
  if (!button || busy || finished || !engine || !gameStarted) return;
  if (fen.split(' ')[1] !== playerColor) return;
  const target = button.dataset.square;
  const squares = boardFromFen(fen);
  const row = 8 - Number(target[1]);
  const col = target.charCodeAt(0) - 97;
  const piece = squares[row * 8 + col];

  if (selected) {
    void chooseMoveForTarget(selected, target).then(move => {
      if (move && !busy && !finished) void playHumanMove(move);
    });
    if (piece && isPlayerPiece(piece)) {
      selected = target;
      renderBoard();
    }
    return;
  }
  if (piece && isPlayerPiece(piece)) selected = target;
  else selected = null;
  renderBoard();
}

boardEl.addEventListener('click', handleSquareClick);
depthEl.addEventListener('input', () => { depthValueEl.textContent = depthEl.value; });
document.querySelector('#new-game').addEventListener('click', openSideModal);
undoButton.addEventListener('click', () => {
  if (busy || turnSnapshots.length === 0) return;
  const snapshot = turnSnapshots.pop();
  fen = snapshot.fen;
  moveLog = snapshot.moveLog;
  lastMove = snapshot.lastMove;
  selected = null;
  finished = false;
  busy = false;
  refreshGame();
  finishIfNeeded();
});
document.querySelector('#copy-fen').addEventListener('click', async () => {
  try {
    await navigator.clipboard.writeText(fen);
    setStatus('FEN copied to clipboard.');
  } catch {
    setStatus('Clipboard access was blocked by the browser.', false);
  }
});
document.querySelectorAll('.side-choice').forEach(button => {
  button.addEventListener('click', () => startGame(button.dataset.side));
});
document.querySelectorAll('[data-promotion]').forEach(button => {
  button.addEventListener('click', () => closePromotion(button.dataset.promotion));
});
promotionModal.addEventListener('click', event => {
  if (event.target === promotionModal) closePromotion(null);
});

async function boot() {
  try {
    engineStateEl.textContent = 'Loading WebAssembly…';
    modalEngineStatus.textContent = 'Downloading KnightForge WebAssembly…';
    setStatus('Downloading the KnightForge WebAssembly engine…', false);
    const module = await import('./engine/knightforge.js');
    const createKnightForge = module.default;
    engine = await createKnightForge();
    engineStateEl.textContent = 'Ready to play';
    modalEngineStatus.textContent = 'Engine ready — choose a color.';
    document.querySelectorAll('.side-choice').forEach(button => { button.disabled = false; });
    busy = true;
    openSideModal();
    refreshGame();
    setStatus('Choose White or Black to begin.', false);
  } catch (error) {
    engineStateEl.textContent = 'Engine unavailable';
    modalEngineStatus.textContent = 'Could not load the engine. Check the Pages deployment.';
    setStatus('Could not load KnightForge. Check the Pages deployment and engine build.', false);
    console.error(error);
  }
}

void boot();
