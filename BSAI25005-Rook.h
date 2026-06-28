#pragma once
#include "BSAI25005-Piece.h"
#include "BSAI25005-Board.h"

class Queen : public Piece {
public:
    Queen(Position p, PieceColor c, Board* b, Texture2D t) : Piece(p, c, b, t) {}
    PieceType getType() const override { return QUEEN; }
    Piece* clone(Board* nb) const override { return new Queen(pos, color, nb, texture); }

    bool isLegal(Position d) override {
        bool straight = isHorizontal(pos, d) or isVertical(pos, d);
        bool diag = isDiagonal(pos, d);
        if (!straight and !diag) return false;
        int rs = (d.row > pos.row) ? 1 : (d.row < pos.row ? -1 : 0);
        int cs = (d.col > pos.col) ? 1 : (d.col < pos.col ? -1 : 0);
        for (int r = pos.row + rs, c = pos.col + cs; r != d.row or c != d.col; r += rs, c += cs)
            if (board->grid[r][c]) return false;
        Piece* t = board->getPiece(d);
        return !(t and t->getColor() == color);
    }
};