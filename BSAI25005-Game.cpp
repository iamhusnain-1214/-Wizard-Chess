#pragma once
#include "raylib.h"
#include "BSAI25005-Globals.h"
#include "BSAI25005-Position.h"

class Piece;
struct BoardSnapshot {
    Piece* grid[8][8];
    PieceColor turn;
    bool       enPassantAvailable;
    Position   enPassantTarget;
    bool       wKingMoved, bKingMoved;
    bool       wRookL, wRookR, bRookL, bRookR;
    int        freezeTurns[8][8];
    int        whiteFreezeCharges, blackFreezeCharges;
};

class Board {
public:
    Piece* grid[8][8];
    PieceColor turn;
    Sound moveSound;
    Sound captureSound;
    bool     enPassantAvailable = false;
    Position enPassantTarget = { -1, -1 };
    bool     wKingMoved = false, bKingMoved = false;
    bool     wRookL = false, wRookR = false, bRookL = false, bRookR = false;
    int freezeTurns[8][8];
    int whiteFreezeCharges = 2, blackFreezeCharges = 2;
    bool     promotionPending = false;
    Position promotionPos = { -1,-1 };
    PieceColor promotionColor = P_NONE;
    Texture2D wp, bp, wr, br, wn, bn, wb, bb, wq, bq, wk, bk;
    Texture2D polyjuice;
    bool      polyjuiceActive = false;
    Board();
    ~Board();
    void   loadTextures();
    void   init();
    void   draw();
    void   clearGrid();
    Piece* getPiece(Position p);
    bool move(Position s, Position d);
    void applyPromotion(PieceType choice);
    bool castFreeze(Position t);
    bool hasAnyLegalMove(PieceColor color);
    bool isInCheck(PieceColor color);
    void handleCastling(Position s, Position d);
    BoardSnapshot takeSnapshot() const;
    void          restoreSnapshot(const BoardSnapshot& snap);
    void          freeSnapshot(BoardSnapshot& snap);
};