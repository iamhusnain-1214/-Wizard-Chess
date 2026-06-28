#include "BSAI25005-Board.h"
#include "BSAI25005-Pawn.h"
#include "BSAI25005-Rook.h"
#include "BSAI25005-Knight.h"
#include "BSAI25005-Bishop.h"
#include "BSAI25005-Queen.h"
#include "BSAI25005-King.h"

Board::Board() {
    turn = CWHITE;
    whiteFreezeCharges = blackFreezeCharges = 2;
    enPassantAvailable = false;
    promotionPending = false;
    polyjuiceActive = false;
    wKingMoved = bKingMoved = wRookL = wRookR = bRookL = bRookR = false;
    for (int i = 0; i < 8; i++) for (int j = 0; j < 8; j++) { grid[i][j] = nullptr; freezeTurns[i][j] = 0; }
}

Board::~Board() {
    clearGrid();
    UnloadTexture(wp); UnloadTexture(bp);
    UnloadTexture(wr); UnloadTexture(br);
    UnloadTexture(wn); UnloadTexture(bn);
    UnloadTexture(wb); UnloadTexture(bb);
    UnloadTexture(wq); UnloadTexture(bq);
    UnloadTexture(wk); UnloadTexture(bk);
}

void Board::loadTextures() {
    wp = LoadTexture("w_pawn.png");   bp = LoadTexture("b_pawn.png");
    wr = LoadTexture("w_rook.png");   br = LoadTexture("b_rook.png");
    wn = LoadTexture("w_knight.png"); bn = LoadTexture("b_knight.png");
    wb = LoadTexture("w_bishop.png"); bb = LoadTexture("b_bishop.png");
    wq = LoadTexture("w_queen.png");  bq = LoadTexture("b_queen.png");
    wk = LoadTexture("w_king.png");   bk = LoadTexture("b_king.png");
    polyjuice = LoadTexture("w_rook.png");
    moveSound = LoadSound("move.mp3");   
    captureSound = LoadSound("kill.mp3");
    SetSoundVolume(captureSound, 1.0f);
    SetSoundVolume(moveSound, 0.7f);
}

void Board::init() {
    loadTextures();
    clearGrid();
    turn = CWHITE;
    whiteFreezeCharges = blackFreezeCharges = 2;
    enPassantAvailable = false;
    promotionPending = false;
    polyjuiceActive = false;
    wKingMoved = bKingMoved = wRookL = wRookR = bRookL = bRookR = false;
    for (int i = 0; i < 8; i++) 
        for (int j = 0; j < 8; j++) 
            freezeTurns[i][j] = 0;

    grid[0][0] = new Rook({ 0,0 }, CBLACK, this, br); grid[0][7] = new Rook({ 0,7 }, CBLACK, this, br);
    grid[7][0] = new Rook({ 7,0 }, CWHITE, this, wr); grid[7][7] = new Rook({ 7,7 }, CWHITE, this, wr);
    grid[0][1] = new Knight({ 0,1 }, CBLACK, this, bn); grid[0][6] = new Knight({ 0,6 }, CBLACK, this, bn);
    grid[7][1] = new Knight({ 7,1 }, CWHITE, this, wn); grid[7][6] = new Knight({ 7,6 }, CWHITE, this, wn);
    grid[0][2] = new Bishop({ 0,2 }, CBLACK, this, bb); grid[0][5] = new Bishop({ 0,5 }, CBLACK, this, bb);
    grid[7][2] = new Bishop({ 7,2 }, CWHITE, this, wb); grid[7][5] = new Bishop({ 7,5 }, CWHITE, this, wb);
    grid[0][3] = new Queen({ 0,3 }, CBLACK, this, bq); grid[0][4] = new King({ 0,4 }, CBLACK, this, bk);
    grid[7][3] = new Queen({ 7,3 }, CWHITE, this, wq); grid[7][4] = new King({ 7,4 }, CWHITE, this, wk);
    for (int i = 0; i < 8; i++) {
        grid[1][i] = new Pawn({ 1,i }, CBLACK, this, bp);
        grid[6][i] = new Pawn({ 6,i }, CWHITE, this, wp);
    }
}

void Board::draw() {
    for (int r = 0; r < 8; r++) for (int c = 0; c < 8; c++)
        if (grid[r][c]) grid[r][c]->draw(TILE_SIZE, BOARD_X, BOARD_Y);
}
bool Board::move(Position s, Position d) {
    Piece* p = grid[s.row][s.col];
    if (!p or p->getColor() != turn) return false;
    if (freezeTurns[s.row][s.col] > 0) return false;
    if (!d.isValid() or !p->isLegal(d)) return false;

    Piece* captured = grid[d.row][d.col];
    Position oldPos = p->getPos();
    if (captured != nullptr) {
        PlaySound(captureSound);
    }
    else {
        PlaySound(moveSound);
    }
    Piece* epCap = nullptr;
    if (dynamic_cast<Pawn*>(p) and enPassantAvailable and d == enPassantTarget) {
        int capRow = (turn == CWHITE) ? d.row + 1 : d.row - 1;
        epCap = grid[capRow][d.col];
        grid[capRow][d.col] = nullptr;
    }

    grid[d.row][d.col] = p; grid[s.row][s.col] = nullptr; p->setPos(d);
    if (isInCheck(turn)) {
        grid[s.row][s.col] = p; grid[d.row][d.col] = captured; p->setPos(oldPos);
        if (epCap) { int cr = (turn == CWHITE) ? d.row + 1 : d.row - 1; grid[cr][d.col] = epCap; }
        return false;
    }    if (captured) delete captured;
    if (epCap)    delete epCap;
    if (dynamic_cast<King*>(p)) { if (turn == CWHITE)wKingMoved = true; else bKingMoved = true; }
    if (dynamic_cast<Rook*>(p)) {
        if (s.col == 0) { if (turn == CWHITE)wRookL = true; else bRookL = true; }
        if (s.col == 7) { if (turn == CWHITE)wRookR = true; else bRookR = true; }
    }
    handleCastling(s, d);
    enPassantAvailable = false;
    if (dynamic_cast<Pawn*>(p) and abs(s.row - d.row) == 2) {
        enPassantAvailable = true;
        enPassantTarget = { (s.row + d.row) / 2,s.col };
    }
    for (int i = 0; i < 8; i++) for (int j = 0; j < 8; j++) if (freezeTurns[i][j] > 0) freezeTurns[i][j]--;
    if (dynamic_cast<Pawn*>(p) and (d.row == 0 or d.row == 7)) {
        promotionPending = true;
        promotionPos = d;
        promotionColor = turn;
        return true;
    }
    turn = (turn == CWHITE) ? CBLACK : CWHITE;
    return true;
}
void Board::applyPromotion(PieceType choice) {
    int r = promotionPos.row, c = promotionPos.col;
    PieceColor col = promotionColor;

    Texture2D tex = {};
    Piece* newPiece = nullptr;
    switch (choice) {
    case QUEEN:  tex = (col == CWHITE) ? wq : bq; newPiece = new Queen({ r,c }, col, this, tex); break;
    case ROOK:   tex = (col == CWHITE) ? wr : br; newPiece = new Rook({ r,c }, col, this, tex); break;
    case BISHOP: tex = (col == CWHITE) ? wb : bb; newPiece = new Bishop({ r,c }, col, this, tex); break;
    case KNIGHT: tex = (col == CWHITE) ? wn : bn; newPiece = new Knight({ r,c }, col, this, tex); break;
    default:     tex = (col == CWHITE) ? wq : bq; newPiece = new Queen({ r,c }, col, this, tex); break;
    }

    delete grid[r][c];
    grid[r][c] = newPiece;

    promotionPending = false;
    promotionPos = { -1,-1 };
    promotionColor = P_NONE;
    turn = (turn == CWHITE) ? CBLACK : CWHITE;
}
void Board::handleCastling(Position s, Position d) {
    Piece* p = grid[d.row][d.col];
    if (!dynamic_cast<King*>(p) or abs(s.col - d.col) != 2) return;
    int origCol = (d.col > s.col) ? 7 : 0;
    int destCol = (d.col > s.col) ? d.col - 1 : d.col + 1;
    Piece* rook = grid[s.row][origCol];
    if (rook) {
        grid[s.row][destCol] = rook; grid[s.row][origCol] = nullptr;
        rook->setPos({ s.row,destCol });
    }
}

bool Board::castFreeze(Position t) {
    if (!t.isValid() or !grid[t.row][t.col]) return false;
    if (turn == CWHITE) {
        if (whiteFreezeCharges <= 0 or grid[t.row][t.col]->getColor() != CBLACK) return false;
        whiteFreezeCharges--;
    }
    else {
        if (blackFreezeCharges <= 0 or grid[t.row][t.col]->getColor() != CWHITE) return false;
        blackFreezeCharges--;
    }
    freezeTurns[t.row][t.col] = 2;
    turn = (turn == CWHITE) ? CBLACK : CWHITE;
    return true;
}

bool Board::hasAnyLegalMove(PieceColor color) {
    for (int r = 0; r < 8; r++) for (int c = 0; c < 8; c++) {
        Piece* p = grid[r][c];
        if (!p or p->getColor() != color) continue;
        for (int r2 = 0; r2 < 8; r2++) for (int c2 = 0; c2 < 8; c2++) {
            Position s = { r,c }, d = { r2,c2 };
            if (!p->isLegal(d)) continue;
            Piece* tmp = grid[d.row][d.col];
            Position old = p->getPos();
            grid[d.row][d.col] = p; grid[s.row][s.col] = nullptr; p->setPos(d);
            bool safe = !isInCheck(color);
            grid[s.row][s.col] = p; grid[d.row][d.col] = tmp; p->setPos(old);
            if (safe) return true;
        }
    }
    return false;
}

Piece* Board::getPiece(Position p) {
    if (p.row < 0 or p.row >= 8 or p.col < 0 or p.col >= 8) return nullptr;
    return grid[p.row][p.col];
}

void Board::clearGrid() {
    for (int i = 0; i < 8; i++) for (int j = 0; j < 8; j++)
        if (grid[i][j]) { delete grid[i][j]; grid[i][j] = nullptr; }
}

bool Board::isInCheck(PieceColor color) {
    Position kp = { -1,-1 };
    for (int r = 0; r < 8; r++) for (int c = 0; c < 8; c++)
        if (grid[r][c] and dynamic_cast<King*>(grid[r][c]) and grid[r][c]->getColor() == color) kp = { r,c };
    if (!kp.isValid()) return false;
    for (int r = 0; r < 8; r++) for (int c = 0; c < 8; c++) {
        Piece* p = grid[r][c];
        if (p and p->getColor() != color and p->isLegal(kp)) return true;
    }
    return false;
}


BoardSnapshot Board::takeSnapshot() const {
    BoardSnapshot snap;
    snap.turn = turn; snap.enPassantAvailable = enPassantAvailable;
    snap.enPassantTarget = enPassantTarget;
    snap.wKingMoved = wKingMoved; snap.bKingMoved = bKingMoved;
    snap.wRookL = wRookL; snap.wRookR = wRookR; snap.bRookL = bRookL; snap.bRookR = bRookR;
    snap.whiteFreezeCharges = whiteFreezeCharges; snap.blackFreezeCharges = blackFreezeCharges;
    for (int i = 0; i < 8; i++) for (int j = 0; j < 8; j++) {
        snap.freezeTurns[i][j] = freezeTurns[i][j];
        snap.grid[i][j] = grid[i][j] ? grid[i][j]->clone(nullptr) : nullptr;
    }
    return snap;
}

void Board::restoreSnapshot(const BoardSnapshot& snap) {
    clearGrid();
    turn = snap.turn; enPassantAvailable = snap.enPassantAvailable;
    enPassantTarget = snap.enPassantTarget;
    wKingMoved = snap.wKingMoved; bKingMoved = snap.bKingMoved;
    wRookL = snap.wRookL; wRookR = snap.wRookR; bRookL = snap.bRookL; bRookR = snap.bRookR;
    whiteFreezeCharges = snap.whiteFreezeCharges; blackFreezeCharges = snap.blackFreezeCharges;
    promotionPending = false;
    for (int i = 0; i < 8; i++) for (int j = 0; j < 8; j++) {
        freezeTurns[i][j] = snap.freezeTurns[i][j];
        grid[i][j] = snap.grid[i][j] ? snap.grid[i][j]->clone(this) : nullptr;
    }
}

void Board::freeSnapshot(BoardSnapshot& snap) {
    for (int i = 0; i < 8; i++) for (int j = 0; j < 8; j++)
        if (snap.grid[i][j]) { delete snap.grid[i][j]; snap.grid[i][j] = nullptr; }
}