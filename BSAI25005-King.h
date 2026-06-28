#pragma once
#include "raylib.h"
#include <string>
#include <vector>

const int SCREEN_W = 1900;
const int SCREEN_H = 1080;
const int TILE_SIZE = 100;
const int BOARD_SIZE = 800;
const int BOARD_X = 80;
const int BOARD_Y = (SCREEN_H - BOARD_SIZE) / 2;
const int RPANEL_X = BOARD_X + BOARD_SIZE + 40;
const int RPANEL_W = SCREEN_W - RPANEL_X - 30;

enum PieceColor { CWHITE, CBLACK, P_NONE };
enum PieceType { PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING, T_NONE };

enum GameState {
    SPLASH, MAIN_MENU, HOUSE_SELECT, PLAYER_NAMES,
    PLAYING, PROMOTION, GAME_OVER             
};
const Color HP_GOLD = { 235, 185,   0, 255 };
const Color HP_DARK_BG = { 10,  10,  20, 255 };
const Color HP_PANEL_BG = { 18,  18,  38, 255 };
const Color HP_PANEL_EDGE = { 50,  45,  80, 255 };
const Color HP_SPELL_GREEN = { 43, 255, 136, 255 };
const Color HP_SPELL_RED = { 255,  60,  60, 255 };
const Color HP_SPELL_BLUE = { 60, 160, 255, 255 };
const Color HP_WHITE_SQ = { 240, 217, 181, 255 };
const Color HP_BLACK_SQ = { 181, 136,  99, 255 };
const Color HP_BORDER = { 80,  70, 120, 255 };
const Color HP_TEXT_DIM = { 160, 150, 180, 255 };
const Color HP_MOVE_DOT = { 80, 220,  80, 160 };
const Color HP_CAPTURE_DOT = { 220,  60,  60, 160 };

const Color GRYFFINDOR_RED = { 174,   0,   1, 255 };
const Color GRYFFINDOR_GOLD = { 235, 185,   0, 255 };
const Color SLYTHERIN_GREEN = { 26, 109,  15, 255 };
const Color SLYTHERIN_SILVER = { 170, 170, 170, 255 };
const Color RAVENCLAW_BLUE = { 14,  26, 143, 255 };
const Color RAVENCLAW_BRONZE = { 148,  90,  29, 255 };
const Color HUFFLEPUFF_YELL = { 236, 185,  57, 255 };
const Color HUFFLEPUFF_BLACK = { 60,  50,  40, 255 };