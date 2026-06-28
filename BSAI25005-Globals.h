#pragma once
#include "raylib.h"
#include "BSAI25005-Board.h"
#include "BSAI25005-Globals.h"
#include <string>
#include <vector>
#include <fstream>

using namespace std;
struct SnapStack {
    static const int CAP = 20;
    BoardSnapshot data[CAP];
    int top = -1;
    bool empty() { return top < 0; }
    bool full() { return top >= CAP - 1; }
    void push(BoardSnapshot s) {
        if (full()) {
            for (int i = 0; i < top; i++)
                data[i] = data[i + 1];
            data[top] = s;
        }
        else {
            data[++top] = s;
        }
    }

    BoardSnapshot pop() { return data[top--]; }

    void clear(Board& b) {
        for (int i = 0; i <= top; i++)
            b.freeSnapshot(data[i]);
        top = -1;
    }
};

enum EffectType { E_NONE, GREEN_FLASH, RED_FLASH, BLUE_FLASH, GOLD_FLASH };
struct Effect { EffectType type; float timer; Position pos; };
enum GameResult { RESULT_NONE, CHECKMATE, STALEMATE };
enum SpellMode { SPELL_NONE, SPELL_FREEZE, SPELL_POLYJUICE, SPELL_EXPECTO, SPELL_WINGARDIUM };
enum House { HOUSE_NONE = -1, GRYFFINDOR = 0, SLYTHERIN = 1, RAVENCLAW = 2, HUFFLEPUFF = 3 };

// Stores one move for the replay system
struct MoveRecord {
    Position from, to;
};

class Game {
private:
    Board     board;
    Position  selected = { -1,-1 };
    Effect    effect = { E_NONE, 0, {-1,-1} };
    GameState state = SPLASH;
    GameResult gameResult = RESULT_NONE;
    Texture2D bgTex;
    Texture2D houseTex[4];
    Font      wizFont, bodyFont;



    float splashTimer = 0, animTimer = 0, msgTimer = 0;
    string statusMsg;
    string player1, player2;
    House  house1 = HOUSE_NONE, house2 = HOUSE_NONE;
    bool   enteringP1 = true;
    int    houseStep = 0;
    bool      showCheck = false;
    bool      spellMode = false;
    SpellMode activeSpell = SPELL_NONE;
    vector<PieceType> whiteCaptured, blackCaptured;
    int wFreeze = 2, bFreeze = 2, wExpecto = 2, bExpecto = 2;
    int wWing = 1, bWing = 1, wPoly = 1, bPoly = 1;
    SnapStack undoStack, redoStack;
    bool vsAI = false;
    bool aiThinking = false;
    float aiTimer = 0.0f;
    void doAIMove();

    // ── Replay system ──────────────────────────────────────────────
    static const int MAX_MOVES = 400;       // max moves we record
    MoveRecord moveHistory[MAX_MOVES];      // every (from,to) played
    int        moveCount = 0;            // how many moves recorded
    bool       isReplaying = false;        // are we in replay mode?
    int        replayIndex = 0;            // which move to play next
    float      replayTimer = 0.0f;        // countdown between moves
    static const float REPLAY_SPEED;       // seconds between each move
    void startReplay();                     // resets board and begins replay
    void updateReplay(float dt);            // called every frame during replay
    void drawReplay();                      // draws "REPLAY" overlay text
    void drawBg();
    void drawSplash();
    void drawMainMenu();
    void drawHouseSelect();
    void drawPlayerNames();
    void drawPlaying();
    void drawPromotion();
    void drawGameOver();
    void drawBoard();
    void drawHighlights();
    void drawEffect();
    void drawRightPanel();
    void panelHeader(int px, int pw, int& cy);
    void panelTurn(int px, int pw, int& cy);
    void panelPlayers(int px, int pw, int& cy);
    void panelCaptured(int px, int pw, int& cy);
    void panelSpells(int px, int pw, int& cy);
    void panelFooter(int px, int pw, int& cy);
    void panelDivider(int px, int pw, int& cy);
    void handleMenu();
    void handleHouseSelect();
    void handlePlayerNames();
    void handlePlaying();
    void handlePromotion();
    void  pushUndo();
    void  doUndo();
    void  doRedo();
    void  saveGame();
    void  loadGame();
    void  setMsg(const char* m, float dur = 2.5f);
    int   charges(SpellMode s, PieceColor c);
    void  spend(SpellMode s, PieceColor c);
    Color       houseColor(House h);
    Color       houseAccent(House h);
    const char* houseName(House h);
    void  roundBox(Rectangle r, Color fill, Color edge, float rnd = 0.1f);
    Color lerp(Color a, Color b, float t);
    Color alpha(Color c, float a);

public:
    Game();
    void init();
    void update();
    void draw();
};