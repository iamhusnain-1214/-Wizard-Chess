#pragma once
#include "BSAI25005-Piece.h"
#include "BSAI25005-Board.h"

class Pawn : public Piece {
public:
    Pawn(Position p, PieceColor c, Board* b, Texture2D t) : Piece(p, c, b, t) {}
    PieceType getType() const override { return PAWN; }
    Piece* clone(Board* nb) const override { return new Pawn(pos, color, nb, texture); }

    bool isLegal(Position d) override {
        int dir = (color == CWHITE) ? -1 : 1;
        if (d.col == pos.col and d.row == pos.row + dir and board->getPiece(d) == nullptr)
            return true;
        int startRow = (color == CWHITE) ? 6 : 1;
        if (pos.row == startRow and d.col == pos.col and d.row == pos.row + 2 * dir) {
            Position mid(pos.row + dir, pos.col);
            if (!board->getPiece(d) and !board->getPiece(mid)) return true;
        }
        if (abs(d.col - pos.col) == 1 and d.row == pos.row + dir) {
            Piece* t = board->getPiece(d);
            if (t and t->getColor() != color) return true;
            if (board->enPassantAvailable and d == board->enPassantTarget) return true;
        }
        return false;
    }
};