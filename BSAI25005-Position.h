#pragma once
#include "BSAI25005-Position.h"
#include "BSAI25005-Globals.h"
#include "raylib.h"

class Board;

class Piece {
protected:
    Position   pos;
    PieceColor color;
    Board* board;
    Texture2D  texture;
public:
    Piece(Position p, PieceColor c, Board* b, Texture2D t)
        : pos(p), color(c), board(b), texture(t) {
    }
    virtual ~Piece() {}

    virtual bool   isLegal(Position dest) = 0;
    virtual Piece* clone(Board* newBoard) const = 0;
    virtual PieceType getType() const = 0;

    virtual void draw(int cellSize, int offX, int offY) {
        DrawTextureEx(texture,
            { (float)(offX + pos.col * cellSize), (float)(offY + pos.row * cellSize) },
            0.0f, (float)cellSize / texture.width, WHITE);
    }
    Texture2D getTexture() const { return texture; }
    Position   getPos()   const { return pos; }
    void       setPos(Position p) { pos = p; }
    PieceColor getColor() const { return color; }
    void       setBoard(Board* b) { board = b; }

    bool isHorizontal(Position a, Position b) const { return a.row == b.row; }
    bool isVertical(Position a, Position b) const { return a.col == b.col; }
    bool isDiagonal(Position a, Position b) const {
        return abs(a.row - b.row) == abs(a.col - b.col);
    }
};