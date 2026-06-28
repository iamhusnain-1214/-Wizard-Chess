#pragma once
#include "BSAI25005-Piece.h"
#include "BSAI25005-Board.h"

class Knight : public Piece {
public:
    Knight(Position p, PieceColor c, Board* b, Texture2D t) : Piece(p, c, b, t) {}
    PieceType getType() const override { return KNIGHT; }
    Piece* clone(Board* nb) const override { return new Knight(pos, color, nb, texture); }

    bool isLegal(Position d) override {
        if (!d.isValid()) return false;
        int dr = abs(pos.row - d.row), dc = abs(pos.col - d.col);
        if (!((dr == 2 and dc == 1) or (dr == 1 and dc == 2))) return false;
        Piece* t = board->getPiece(d);
        return !(t and t->getColor() == color);
    }
};