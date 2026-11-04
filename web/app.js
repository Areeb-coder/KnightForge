import createKnightForge from './engine/knightforge.js';

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

let engine = null;
let fen = START_FEN;
let selected = null;
let legalMoves = [];
let moveLog = [];
let turnSnapshots = [];
let busy = true;
let finished = false;
let lastMove = null;

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

function isWhite(piece) {
  return Boolean(piece && piece === piece.toUpperCase());
}

function renderBoard() {
  const squares = boardFromFen(fen);
  const targets = selected ? legalMoves.filter(move => move.slice(0, 2) === selected).map(move => move.slice(2, 4)) : [];
  boardEl.innerHTML = '';

  for (let index = 0; index < 64; index++) {
    const row = Math.floor(index / 8);
    const col = index % 8;
    const squareName = String.fromCharCode(97 + col) + String(8 - row);
    const piece = squares[index];
    const button = document.createElement('button');
    button.type = 'button';
    button.className = `square ${(row + col) % 2 === 0 ? 'light' : 'dark'}`;
    button.dataset.square = squareName;
    button.setAttribute('aria-label', `${squareName}${piece ? ' ' + piece : ' empty'}`);

    if (squareName === selected) button.classList.add('selected');
    if (lastMove && (squareName === lastMove.slice(0, 2) || squareName === lastMove.slice(2, 4))) button.classList.add('last-move');
    if (targets.includes(squareName)) {
      button.classList.add('legal');
      if (piece) button.classList.add('capture');
    }

    if (piece) {
      const pieceEl = document.createElement('span');
      pieceEl.className = `piece ${isWhite(piece) ? 'white' : 'black'}`;
      pieceEl.textContent = SYMBOLS[piece];
      button.append(pieceEl);
    }

    if (col === 0) {
      const coord = document.createElement('span');
      coord.className = 'coord';
      coord.textContent = String(8 - row);
      button.append(coord);
    } else if (row === 7) {
      const coord = document.createElement('span');
      coord.className = 'coord';
      coord.textContent = String.fromCharCode(97 + col);
      button.append(coord);
    }
    boardEl.append(button);
  }
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
    number.textContent = `${Math.floor(i / 2) + 1}.`;
    const white = document.createElement('span');
    white.className = 'move-white';
    white.textContent = moveLog[i] || '';
    const black = document.createElement('span');
    black.className = 'move-black';
    black.textContent = moveLog[i + 1] || '';
    pair.append(number, white, black);
    movesEl.append(pair);
  }
  moveCountEl.textContent = `${moveLog.length} plies`;
  undoButton.disabled = busy || turnSnapshots.length === 0;
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
  const [san, nextFen] = result.split('\n');
  return { san, fen: nextFen };
}

function getStatus(position) {
  return engine.ccall('knightforge_status', 'string', ['string'], [position]);
}

function finishIfNeeded() {
  const state = getStatus(fen);
  if (state === 'checkmate') {
    const sideToMove = fen.split(' ')[1];
    finished = true;
    setStatus(sideToMove === 'w' ? 'Checkmate — KnightForge wins.' : 'Checkmate — you win!');
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
    setStatus(fen.split(' ')[1] === 'w' ? 'Your king is in check.' : 'KnightForge is in check.');
  } else {
    setStatus(fen.split(' ')[1] === 'w' ? 'Your move — play White.' : 'KnightForge is thinking…', !busy);
  }
  return false;
}

function chooseMoveForTarget(from, to) {
  const candidates = legalMoves.filter(move => move.slice(0, 2) === from && move.slice(2, 4) === to);
  if (candidates.length <= 1) return candidates[0] || null;
  return candidates.find(move => move[4] === 'q') || candidates[0];
}

async function playHumanMove(move) {
  if (busy || finished || !engine) return;
  const before = fen;
  const snapshot = { fen: before, moveLog: [...moveLog], lastMove };
  try {
    const applied = applyMove(fen, move);
    turnSnapshots.push(snapshot);
    fen = applied.fen;
    moveLog.push(applied.san);
    lastMove = move;
    selected = null;
    legalMoves = getLegalMoves(fen);
    refreshGame();

    if (finishIfNeeded()) {
      renderMoves();
      return;
    }

    busy = true;
    engineStateEl.textContent = 'Calculating…';
    setStatus('KnightForge is calculating its reply.', false);
    renderMoves();

    const depth = Number(depthEl.value);
    const time = Number(thinkTimeEl.value);
    const best = engine.ccall('knightforge_best_move', 'string', ['string', 'number', 'number'], [fen, depth, time]);
    if (!best || best === '0000' || best.startsWith('ERROR')) {
      finished = true;
      setStatus('The engine could not find a reply. Start a new game to continue.', false);
      return;
    }

    const response = applyMove(fen, best);
    fen = response.fen;
    moveLog.push(response.san);
    lastMove = best;
    engineStateEl.textContent = 'Ready to play';
    busy = false;
    refreshGame();
    finishIfNeeded();
    renderMoves();
  } catch (error) {
    busy = false;
    setStatus(`Game error: ${error.message}`, false);
    console.error(error);
  }
}

function handleSquareClick(event) {
  const button = event.target.closest('button[data-square]');
  if (!button || busy || finished || !engine) return;
  const target = button.dataset.square;
  const squares = boardFromFen(fen);
  const row = 8 - Number(target[1]);
  const col = target.charCodeAt(0) - 97;
  const piece = squares[row * 8 + col];

  if (selected) {
    const move = chooseMoveForTarget(selected, target);
    if (move) {
      void playHumanMove(move);
      return;
    }
  }
  if (piece && isWhite(piece)) {
    selected = target;
  } else {
    selected = null;
  }
  renderBoard();
}

function newGame() {
  if (!engine) return;
  fen = START_FEN;
  selected = null;
  moveLog = [];
  turnSnapshots = [];
  lastMove = null;
  busy = false;
  finished = false;
  engineStateEl.textContent = 'Ready to play';
  refreshGame();
  setStatus('New game. You play White.');
}

function undoTurn() {
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
}

boardEl.addEventListener('click', handleSquareClick);
depthEl.addEventListener('input', () => { depthValueEl.textContent = depthEl.value; });
document.querySelector('#new-game').addEventListener('click', newGame);
undoButton.addEventListener('click', undoTurn);
document.querySelector('#copy-fen').addEventListener('click', async () => {
  try {
    await navigator.clipboard.writeText(fen);
    setStatus('FEN copied to clipboard.');
  } catch {
    setStatus('Clipboard access was blocked by the browser.', false);
  }
});

async function boot() {
  try {
    engineStateEl.textContent = 'Loading WebAssembly…';
    setStatus('Downloading the KnightForge WebAssembly engine…', false);
    engine = await createKnightForge();
    busy = false;
    engineStateEl.textContent = 'Ready to play';
    refreshGame();
    setStatus('Engine ready. You play White.');
  } catch (error) {
    engineStateEl.textContent = 'Engine unavailable';
    setStatus('Could not load KnightForge. Check the Pages deployment and engine build.', false);
    console.error(error);
  }
}

void boot();
