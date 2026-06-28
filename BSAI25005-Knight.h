#pragma once
#include "BSAI25005-Piece.h"

class King : public Piece {
public:
    King(Position p, PieceColor c, Board* b, Texture2D t) : Piece(p, c, b, t) {}
    PieceType getType() const override { return KING; }
    Piece* clone(Board* nb) const override { return new King(pos, color, nb, texture); }

    bool isLegal(Position d) override {
        int dr = abs(pos.row - d.row), dc = abs(pos.col - d.col);
        if (dr == 0 and dc == 2) {
            if (color == CWHITE and board->wKingMoved) return false;
            if (color == CBLACK and board->bKingMoved) return false;
            int rookCol = (d.col > pos.col) ? 7 : 0;
            if (rookCol == 0 and color == CWHITE and board->wRookL) return false;
            if (rookCol == 7 and color == CWHITE and board->wRookR) return false;
            if (rookCol == 0 and color == CBLACK and board->bRookL) return false;
            if (rookCol == 7 and color == CBLACK and board->bRookR) return false;
            Piece* rook = board->grid[pos.row][rookCol];
            if (!rook or rook->getColor() != color) return false;
            int step = (d.col > pos.col) ? 1 : -1;
            for (int c = pos.col + step; c != rookCol; c += step)
                if (board->grid[pos.row][c]) return false;
            if (board->isInCheck(color)) return false;
            int passCol = pos.col + step;
            Piece* tmp = board->grid[pos.row][passCol];
            board->grid[pos.row][passCol] = const_cast<King*>(this);
            board->grid[pos.row][pos.col] = nullptr;
            bool bad = board->isInCheck(color);
            board->grid[pos.row][pos.col] = const_cast<King*>(this);
            board->grid[pos.row][passCol] = tmp;
            return !bad;
        }
        if (dr <= 1 and dc <= 1 and (dr + dc) > 0) {
            if (!d.isValid()) return false;
            Piece* t = board->getPiece(d);
            return !(t and t->getColor() == color);
        }
        return false;
    }
};