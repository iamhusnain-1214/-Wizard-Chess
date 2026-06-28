#pragma once
struct Position {
    int row;
    int col;
    Position(int r = -1, int c = -1) : row(r), col(c) {}
    bool operator==(const Position& other) const {
        return (row == other.row and col == other.col);
    }
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
    bool isValid() const {
        return (row >= 0 and row < 8 and col >= 0 and col < 8);
    }
};