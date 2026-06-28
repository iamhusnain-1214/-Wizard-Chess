#pragma once
#include "BSAI25005-Piece.h"
#include "BSAI25005-Board.h"

class Bishop : public Piece {
public:
    Bishop(Position p, PieceColor c, Board* b, Texture2D t) : Piece(p, c, b, t) {}
    PieceType getType() const override { 
        return BISHOP;
    }
    Piece* clone(Board* nb) const override {
        return new Bishop(pos, color, nb, texture);
    }

    bool isLegal(Position d) override {
        if (!isDiagonal(pos, d))
            return false;
        int rs = (d.row > pos.row) ? 1 : -1, cs = (d.col > pos.col) ? 1 : -1;
        for (int r = pos.row + rs, c = pos.col + cs; r != d.row or c != d.col; r += rs, c += cs)
            if (board->grid[r][c]) 
                return false;
        Piece* t = board->getPiece(d);
        return !(t and t->getColor() == color);
    }
};