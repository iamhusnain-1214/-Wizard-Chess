#include "BSAI25005-Game.h"
#include "BSAI25005-Piece.h"
#include "BSAI25005-King.h"
#include "BSAI25005-Queen.h"
#include "BSAI25005-Rook.h"
#include "BSAI25005-Bishop.h"
#include "BSAI25005-Knight.h"
#include "BSAI25005-Pawn.h"
#include <cmath>
#include <cstdio>

Color Game::lerp(Color a, Color b, float t) {
    return {
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        (unsigned char)(a.a + (b.a - a.a) * t)
    };
}
Color Game::alpha(Color c, float a) {
    return { c.r, c.g, c.b, (unsigned char)(a * 255) };
}
void Game::roundBox(Rectangle r, Color fill, Color edge, float rnd) {
    DrawRectangleRounded(r, rnd, 8, fill);
    DrawRectangleRoundedLines(r, rnd, 8, edge);
}

void Game::setMsg(const char* m, float d) {
    statusMsg = m;
    msgTimer = d;
}
int Game::charges(SpellMode s, PieceColor c) {
    bool w = (c == CWHITE);
    if (s == SPELL_FREEZE)    return w ? wFreeze : bFreeze;
    if (s == SPELL_EXPECTO)   return w ? wExpecto : bExpecto;
    if (s == SPELL_WINGARDIUM)return w ? wWing : bWing;
    if (s == SPELL_POLYJUICE) return w ? wPoly : bPoly;
    return 0;
}
void Game::spend(SpellMode s, PieceColor c) {
    bool w = (c == CWHITE);
    if (s == SPELL_FREEZE) { if (w) wFreeze--;  else bFreeze--; }
    if (s == SPELL_EXPECTO) { if (w) wExpecto--; else bExpecto--; }
    if (s == SPELL_WINGARDIUM) { if (w) wWing--;    else bWing--; }
    if (s == SPELL_POLYJUICE) { if (w) wPoly--;    else bPoly--; }
}

Color Game::houseColor(House h) {
    if (h == GRYFFINDOR) return GRYFFINDOR_RED;
    if (h == SLYTHERIN)  return SLYTHERIN_GREEN;
    if (h == RAVENCLAW)  return RAVENCLAW_BLUE;
    if (h == HUFFLEPUFF) return HUFFLEPUFF_YELL;
    return HP_TEXT_DIM;
}

Color Game::houseAccent(House h) {
    if (h == GRYFFINDOR) return GRYFFINDOR_GOLD;
    if (h == SLYTHERIN)  return SLYTHERIN_SILVER;
    if (h == RAVENCLAW)  return RAVENCLAW_BRONZE;
    if (h == HUFFLEPUFF) return HUFFLEPUFF_BLACK;
    return HP_TEXT_DIM;
}

const char* Game::houseName(House h) {
    if (h == GRYFFINDOR) return "Gryffindor";
    if (h == SLYTHERIN)  return "Slytherin";
    if (h == RAVENCLAW)  return "Ravenclaw";
    if (h == HUFFLEPUFF) return "Hufflepuff";
    return "None";
}
void Game::pushUndo() {
    if (undoStack.full()) {
        board.freeSnapshot(undoStack.data[0]);
        for (int i = 0; i < undoStack.top; i++)
            undoStack.data[i] = undoStack.data[i + 1];
        undoStack.top--;
    }
    undoStack.push(board.takeSnapshot());
    redoStack.clear(board);
}

void Game::doUndo() {
    if (undoStack.empty()) { setMsg("Nothing to undo!"); return; }
    redoStack.push(board.takeSnapshot());
    BoardSnapshot snap = undoStack.pop();
    board.restoreSnapshot(snap);
    board.freeSnapshot(snap);
    wFreeze = board.whiteFreezeCharges;
    bFreeze = board.blackFreezeCharges;
    setMsg("Undo!");
}

void Game::doRedo() {
    if (redoStack.empty()) { setMsg("Nothing to redo!"); return; }
    undoStack.push(board.takeSnapshot());
    BoardSnapshot snap = redoStack.pop();
    board.restoreSnapshot(snap);
    board.freeSnapshot(snap);
    wFreeze = board.whiteFreezeCharges;
    bFreeze = board.blackFreezeCharges;
    setMsg("Redo!");
}


void Game::saveGame() {
    ofstream f("save.txt");
    if (!f) { setMsg("Save failed!"); return; }

    f << board.turn << "\n";
    f << board.wKingMoved << " " << board.bKingMoved << "\n";
    f << board.wRookL << " " << board.wRookR << "\n";
    f << board.bRookL << " " << board.bRookR << "\n";
    f << board.enPassantAvailable << " "
        << board.enPassantTarget.row << " "
        << board.enPassantTarget.col << "\n";
    f << board.whiteFreezeCharges << " " << board.blackFreezeCharges << "\n";
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            f << board.freezeTurns[r][c] << " ";
    f << "\n";
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            Piece* p = board.grid[r][c];
            if (p)
                f << r << " " << c << " " << p->getType() << " " << p->getColor() << "\n";
        }
    }
    f << "-1\n";
    f << whiteCaptured.size() << "\n";
    for (PieceType pt : whiteCaptured) f << pt << " ";
    f << "\n";
    f << blackCaptured.size() << "\n";
    for (PieceType pt : blackCaptured) f << pt << " ";
    f << "\n";
    f << wFreeze << " " << bFreeze << " "
        << wExpecto << " " << bExpecto << " "
        << wWing << " " << bWing << " "
        << wPoly << " " << bPoly << "\n";

    setMsg("Game saved!");
}

void Game::loadGame() {
    ifstream f("save.txt");
    if (!f) { setMsg("No save found!"); return; }

    board.clearGrid();

    int turn; f >> turn; board.turn = (PieceColor)turn;

    f >> board.wKingMoved >> board.bKingMoved;
    f >> board.wRookL >> board.wRookR;
    f >> board.bRookL >> board.bRookR;

    int epAvail, epRow, epCol;
    f >> epAvail >> epRow >> epCol;
    board.enPassantAvailable = epAvail;
    board.enPassantTarget = { epRow, epCol };

    f >> board.whiteFreezeCharges >> board.blackFreezeCharges;

    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            f >> board.freezeTurns[r][c];

    int r, c, pt, pc;
    while (f >> r and r != -1) {
        f >> c >> pt >> pc;
        PieceType  type = (PieceType)pt;
        PieceColor color = (PieceColor)pc;
        Texture2D tex = {};
        if (type == PAWN)  tex = (color == CWHITE) ? board.wp : board.bp;
        if (type == ROOK)  tex = (color == CWHITE) ? board.wr : board.br;
        if (type == KNIGHT)tex = (color == CWHITE) ? board.wn : board.bn;
        if (type == BISHOP)tex = (color == CWHITE) ? board.wb : board.bb;
        if (type == QUEEN) tex = (color == CWHITE) ? board.wq : board.bq;
        if (type == KING)  tex = (color == CWHITE) ? board.wk : board.bk;

        if (type == PAWN)  board.grid[r][c] = new Pawn({ r,c }, color, &board, tex);
        if (type == ROOK)  board.grid[r][c] = new Rook({ r,c }, color, &board, tex);
        if (type == KNIGHT)board.grid[r][c] = new Knight({ r,c }, color, &board, tex);
        if (type == BISHOP)board.grid[r][c] = new Bishop({ r,c }, color, &board, tex);
        if (type == QUEEN) board.grid[r][c] = new Queen({ r,c }, color, &board, tex);
        if (type == KING)  board.grid[r][c] = new King({ r,c }, color, &board, tex);
    }
    int ws; f >> ws;
    whiteCaptured.resize(ws);
    for (int i = 0; i < ws; i++) { int x; f >> x; whiteCaptured[i] = (PieceType)x; }

    int bs; f >> bs;
    blackCaptured.resize(bs);
    for (int i = 0; i < bs; i++) { int x; f >> x; blackCaptured[i] = (PieceType)x; }

    f >> wFreeze >> bFreeze >> wExpecto >> bExpecto >> wWing >> bWing >> wPoly >> bPoly;

    setMsg("Game loaded!");
    state = PLAYING;
}
const float Game::REPLAY_SPEED = 0.8f; 
void Game::startReplay() {
    board.init();
    whiteCaptured.clear();
    blackCaptured.clear();
    wFreeze = bFreeze = 2;
    wExpecto = bExpecto = 2;
    wWing = bWing = wPoly = bPoly = 1;
    selected = { -1, -1 };
    isReplaying = true;
    replayIndex = 0;
    replayTimer = REPLAY_SPEED;
    state = PLAYING;
    setMsg("Replaying moves...", 2.0f);
}

void Game::updateReplay(float dt) {
    replayTimer -= dt;
    if (replayTimer > 0.0f)
        return;   
    if (replayIndex >= moveCount) {
        isReplaying = false;
        setMsg("Replay finished!", 2.5f);
        return;
    }
    MoveRecord& m = moveHistory[replayIndex];
    effect = { GREEN_FLASH, 0.4f, m.to };
    board.move(m.from, m.to);          
    replayIndex++;
    replayTimer = REPLAY_SPEED;        
}

void Game::drawReplay() {
    DrawRectangle(0, 0, SCREEN_W, 52, alpha(HP_DARK_BG, 0.85f));

    const char* tag = "REPLAY";
    Vector2 ts = MeasureTextEx(wizFont, tag, 38, 1);
    DrawTextEx(wizFont, tag,
        { (SCREEN_W - ts.x) / 2.0f, 8 }, 38, 1, HP_GOLD);
    char counter[40];
    snprintf(counter, sizeof(counter), "Move %d / %d", replayIndex, moveCount);
    Vector2 cs = MeasureTextEx(bodyFont, counter, 24, 1);
    DrawTextEx(bodyFont, counter,
        { (SCREEN_W - cs.x) / 2.0f + ts.x / 2.0f + 20, 14 },
        24, 1, HP_TEXT_DIM);
}
void Game::doAIMove() {
    struct Move { Position from, to; };
    Move captureMoves[64]; 
    int  captureCount = 0;
    Move normalMoves[200]; 
    int  normalCount = 0;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            Piece* p = board.grid[r][c];
            if (!p or p->getColor() != CBLACK) continue;
            if (board.freezeTurns[r][c] > 0)   continue;  

            Position from = { r, c };
            for (int r2 = 0; r2 < 8; r2++) {
                for (int c2 = 0; c2 < 8; c2++) {
                    Position to = { r2, c2 };
                    if (!p->isLegal(to)) continue;

                    Piece* saved = board.grid[r2][c2];
                    board.grid[r2][c2] = p;
                    board.grid[r][c] = nullptr;
                    p->setPos(to);

                    bool inCheck = board.isInCheck(CBLACK);

                    board.grid[r][c] = p;
                    board.grid[r2][c2] = saved;
                    p->setPos(from);

                    if (inCheck) continue; 
                    if (saved != nullptr) {
                        if (captureCount < 64)
                            captureMoves[captureCount++] = { from, to };
                    }
                    else {
                        if (normalCount < 200)
                            normalMoves[normalCount++] = { from, to };
                    }
                }
            }
        }
    }
    Move chosen = { {-1,-1},{-1,-1} };
    if (captureCount > 0) {
        int idx = GetRandomValue(0, captureCount - 1);
        chosen = captureMoves[idx];
    }
    else if (normalCount > 0) {
        int idx = GetRandomValue(0, normalCount - 1);
        chosen = normalMoves[idx];
    }

    if (chosen.from.isValid()) {
        pushUndo();
        Piece* dest = board.getPiece(chosen.to);
        PieceType captType = dest ? dest->getType() : T_NONE;
        bool moved = board.move(chosen.from, chosen.to);
        if (moved) {
            if (moveCount < MAX_MOVES) {
                moveHistory[moveCount].from = chosen.from;
                moveHistory[moveCount].to = chosen.to;
                moveCount++;
            }
            if (captType != T_NONE) {
                effect = { GREEN_FLASH, 0.5f, chosen.to };
                blackCaptured.push_back(captType);
            }
            else {
                effect = { GREEN_FLASH, 0.25f, chosen.to };
            }
        }
        else {
            board.freeSnapshot(undoStack.data[undoStack.top]);
            undoStack.top--;
        }
    }
}

Game::Game() {}

void Game::init() {
    board.init();

    wizFont = LoadFontEx("Myfont.ttf", 72, nullptr, 0);
    bodyFont = LoadFontEx("Myfont.ttf", 36, nullptr, 0);
    bgTex = LoadTexture("final_bg.png");

    houseTex[0] = LoadTexture("crest_gryffindor.png");
    houseTex[1] = LoadTexture("crest_slytherin.png");
    houseTex[2] = LoadTexture("crest_ravenclaw.png");
    houseTex[3] = LoadTexture("crest_hufflepuff.png");

    player1 = player2 = "";
    house1 = house2 = HOUSE_NONE;
    enteringP1 = true;
    houseStep = 0;
    vsAI = false;
    aiThinking = false;
    aiTimer = 0.0f;
    moveCount = 0;
    isReplaying = false;
    replayIndex = 0;
    replayTimer = 0.0f;

    whiteCaptured.clear();
    blackCaptured.clear();
    spellMode = false;
    activeSpell = SPELL_NONE;

    splashTimer = animTimer = msgTimer = 0;
    wFreeze = bFreeze = 2;
    wExpecto = bExpecto = 2;
    wWing = bWing = wPoly = bPoly = 1;

    undoStack.clear(board);
    redoStack.clear(board);
}

void Game::update() {
    float dt = GetFrameTime();
    animTimer += dt;
    if (state == SPLASH) {
        splashTimer += dt;
        if (splashTimer > 3.5f or IsKeyPressed(KEY_SPACE) or IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            state = MAIN_MENU;
        return;
    }

    if (state == MAIN_MENU) { handleMenu();        return; }
    if (state == HOUSE_SELECT) { handleHouseSelect(); return; }
    if (state == PLAYER_NAMES) { handlePlayerNames(); return; }
    if (state == PROMOTION) { handlePromotion();   return; }

    if (state == PLAYING) {
        if (isReplaying) {
            updateReplay(dt);
            return;
        }
        if (vsAI and board.turn == CBLACK and !board.promotionPending) {
            if (!aiThinking) {
                aiThinking = true;
                aiTimer = 0.4f; 
                setMsg("AI is thinking...", 0.5f);
            }
            else {
                aiTimer -= dt;
                if (aiTimer <= 0.0f) {
                    doAIMove();
                    aiThinking = false;
                }
            }
        }
        handlePlaying();
        if (board.promotionPending) { state = PROMOTION; return; }
        showCheck = board.isInCheck(board.turn);
        bool hasMove = board.hasAnyLegalMove(board.turn);
        if (!hasMove and showCheck) { gameResult = CHECKMATE; state = GAME_OVER; }
        if (!hasMove and !showCheck) { gameResult = STALEMATE; state = GAME_OVER; }
    }

    if (state == GAME_OVER) {
        if (IsKeyPressed(KEY_R)) {
            board.init();
            whiteCaptured.clear();
            blackCaptured.clear();
            selected = { -1,-1 };
            spellMode = false;
            activeSpell = SPELL_NONE;
            gameResult = RESULT_NONE;
            showCheck = false;
            wFreeze = bFreeze = 2;
            wExpecto = bExpecto = 2;
            wWing = bWing = wPoly = bPoly = 1;
            undoStack.clear(board);
            redoStack.clear(board);
            moveCount = 0;  
            isReplaying = false;
            state = PLAYING;
        }
        if (IsKeyPressed(KEY_T)) startReplay();   
        if (IsKeyPressed(KEY_M)) state = MAIN_MENU;
    }
    if (effect.timer > 0) { effect.timer -= dt; if (effect.timer <= 0) effect.type = E_NONE; }
    if (msgTimer > 0) { msgTimer -= dt; if (msgTimer <= 0) statusMsg = ""; }
}

void Game::draw() {
    ClearBackground(HP_DARK_BG);
    drawBg();
    if (state == SPLASH)       drawSplash();
    else if (state == MAIN_MENU)    drawMainMenu();
    else if (state == HOUSE_SELECT) drawHouseSelect();
    else if (state == PLAYER_NAMES) drawPlayerNames();
    else if (state == PLAYING)      drawPlaying();
    else if (state == PROMOTION) { drawPlaying(); drawPromotion(); }
    else if (state == GAME_OVER)    drawGameOver();
}

void Game::drawBg() {
    if (bgTex.id > 0)
        DrawTexturePro(bgTex, { 0,0,(float)bgTex.width,(float)bgTex.height },
            { 0,0,(float)SCREEN_W,(float)SCREEN_H }, { 0,0 }, 0, WHITE);
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, alpha(HP_DARK_BG, 0.60f));
    for (int i = 0; i < 60; i++) {
        float x = (float)((i * 317 + 89) % SCREEN_W);
        float y = (float)((i * 211 + 43) % SCREEN_H);
        float b = 0.2f + 0.6f * sinf(animTimer * 1.2f + i * 0.9f);
        DrawPixel((int)x, (int)y, alpha(WHITE, b * 0.55f));
    }
}

void Game::drawSplash() {
    float p = 0.5f + 0.5f * sinf(animTimer * 2.0f);
    DrawCircle(SCREEN_W / 2, SCREEN_H / 2 - 60, (int)(180 + 20 * p), alpha(HP_GOLD, 0.06f * p));
    DrawCircle(SCREEN_W / 2, SCREEN_H / 2 - 60, (int)(140 + 10 * p), alpha(HP_GOLD, 0.10f * p));
    const char* t = "WIZARD CHESS";
    Vector2 ts = MeasureTextEx(wizFont, t, 90, 2);
    DrawTextEx(wizFont, t, { (SCREEN_W - ts.x) / 2.0f + 3, SCREEN_H / 2.0f - 97 }, 90, 2, alpha(HP_GOLD, 0.3f * p));
    DrawTextEx(wizFont, t, { (SCREEN_W - ts.x) / 2.0f,   SCREEN_H / 2.0f - 100 }, 90, 2, HP_GOLD);
    const char* s = "Hogwarts Edition";
    Vector2 ss = MeasureTextEx(bodyFont, s, 36, 1);
    DrawTextEx(bodyFont, s, { (SCREEN_W - ss.x) / 2.0f, SCREEN_H / 2.0f + 20 }, 36, 1, HP_TEXT_DIM);
    if ((int)(animTimer * 2) % 2 == 0) {
        const char* pr = "Press SPACE to enter Hogwarts";
        Vector2 ps = MeasureTextEx(bodyFont, pr, 28, 1);
        DrawTextEx(bodyFont, pr, { (SCREEN_W - ps.x) / 2.0f, SCREEN_H / 2.0f + 100 }, 28, 1, alpha(HP_GOLD, 0.7f));
    }
}


void Game::drawMainMenu() {
    float p = 0.5f + 0.5f * sinf(animTimer * 1.8f);

    const char* t = "WIZARD CHESS";
    Vector2 ts = MeasureTextEx(wizFont, t, 100, 2);
    DrawTextEx(wizFont, t, { (SCREEN_W - ts.x) / 2.0f + 3, (float)(SCREEN_H * 0.18) + 3 }, 100, 2, alpha(HP_GOLD, 0.25f * p));
    DrawTextEx(wizFont, t, { (SCREEN_W - ts.x) / 2.0f,   (float)(SCREEN_H * 0.18) }, 100, 2, HP_GOLD);

    const char* sub = "Hogwarts Edition";
    Vector2 ss = MeasureTextEx(bodyFont, sub, 34, 1);
    DrawTextEx(bodyFont, sub, { (SCREEN_W - ss.x) / 2.0f, (float)(SCREEN_H * 0.18 + 110) }, 34, 1, HP_TEXT_DIM);

    int dy = (int)(SCREEN_H * 0.38f);
    DrawLine(SCREEN_W / 2 - 300, dy, SCREEN_W / 2 + 300, dy, alpha(HP_GOLD, 0.35f));
    DrawRectangle(SCREEN_W / 2 - 5, dy - 5, 10, 10, alpha(HP_GOLD, 0.5f));
    struct Btn { const char* label; float yPos; Color col; };
    Btn btns[] = {
        { "2 PLAYERS",  0.44f, HP_GOLD },
        { "VS AI",      0.54f, HP_SPELL_GREEN },
        { "LOAD GAME",  0.64f, HP_SPELL_BLUE },
        { "EXIT",       0.74f, HP_SPELL_RED }
    };

    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < 4; i++) {
        Rectangle r = { (SCREEN_W - 360.0f) / 2, SCREEN_H * btns[i].yPos - 30, 360, 60 };
        bool hov = CheckCollisionPointRec(mouse, r);
        roundBox(r, alpha(hov ? btns[i].col : Color{ 30,20,50,255 }, 0.85f),
            hov ? btns[i].col : alpha(btns[i].col, 0.45f), 0.3f);
        if (hov) DrawRectangleRounded(r, 0.3f, 8, alpha(btns[i].col, 0.12f));
        Vector2 ls = MeasureTextEx(wizFont, btns[i].label, 36, 1);
        DrawTextEx(wizFont, btns[i].label,
            { r.x + (r.width - ls.x) / 2, r.y + (r.height - ls.y) / 2 },
            36, 1, hov ? btns[i].col : WHITE);
    }
}
void Game::drawHouseSelect() {
    bool isP1 = (houseStep == 0);

    char hdr[80];
    snprintf(hdr, sizeof(hdr), "%s — Choose Your House", isP1 ? "PLAYER 1" : "PLAYER 2");
    Vector2 hs = MeasureTextEx(wizFont, hdr, 52, 1);
    DrawTextEx(wizFont, hdr, { (SCREEN_W - hs.x) / 2.0f, 55 }, 52, 1, HP_GOLD);
    const char* pn = isP1 ? player1.c_str() : player2.c_str();
    if (strlen(pn)) {
        char sub[64]; snprintf(sub, sizeof(sub), "\"%s\"", pn);
        Vector2 ps = MeasureTextEx(bodyFont, sub, 30, 1);
        DrawTextEx(bodyFont, sub, { (SCREEN_W - ps.x) / 2.0f, 120 }, 30, 1, HP_TEXT_DIM);
    }
    const char* names[4] = { "Gryffindor","Slytherin","Ravenclaw","Hufflepuff" };
    const char* motto[4] = { "Brave at Heart","Cunning & Ambitious","Wit & Wisdom","Patient & Loyal" };
    Color pri[4] = { GRYFFINDOR_RED, SLYTHERIN_GREEN, RAVENCLAW_BLUE, HUFFLEPUFF_YELL };
    Color acc[4] = { GRYFFINDOR_GOLD,SLYTHERIN_SILVER,RAVENCLAW_BRONZE,HUFFLEPUFF_BLACK };

    int cw = 330, ch = 450, gap = 40;
    int sx = (SCREEN_W - (4 * cw + 3 * gap)) / 2;
    int startY = (SCREEN_H - ch) / 2 + 20;
    Vector2 mouse = GetMousePosition();

    for (int i = 0; i < 4; i++) {
        Rectangle card = { (float)(sx + i * (cw + gap)), (float)startY, (float)cw, (float)ch };
        bool hov = CheckCollisionPointRec(mouse, card);
        bool sel = (isP1 ? (int)house1 : (int)house2) == i;

        if (hov) card.y -= 8;
        DrawRectangleRounded({ card.x + 5, card.y + 7, card.width, card.height }, 0.08f, 8, alpha(BLACK, 0.5f));
        roundBox(card, HP_PANEL_BG, sel ? acc[i] : (hov ? pri[i] : alpha(HP_BORDER, 0.5f)), 0.08f);
        DrawRectangleRounded({ card.x, card.y, (float)cw, 80 }, 0.08f, 8, alpha(pri[i], 0.8f));
        if (houseTex[i].id > 0) {
            float sc = 110.0f / houseTex[i].width;
            DrawTextureEx(houseTex[i], { card.x + (cw - 110) / 2.0f, card.y + 88 }, 0, sc, WHITE);
        }
        else {
            DrawCircle((int)(card.x + cw / 2), (int)(card.y + 150), 50, alpha(pri[i], 0.3f));
            DrawCircleLines((int)(card.x + cw / 2), (int)(card.y + 150), 50, pri[i]);
            char ini[2] = { names[i][0], 0 };
            Vector2 is2 = MeasureTextEx(wizFont, ini, 55, 1);
            DrawTextEx(wizFont, ini, { card.x + (cw - is2.x) / 2.0f, card.y + 123 }, 55, 1, acc[i]);
        }
        Vector2 ns = MeasureTextEx(wizFont, names[i], 32, 1);
        DrawTextEx(wizFont, names[i], { card.x + (cw - ns.x) / 2.0f, card.y + 250 }, 32, 1, acc[i]);
        Vector2 ms2 = MeasureTextEx(bodyFont, motto[i], 20, 1);
        DrawTextEx(bodyFont, motto[i], { card.x + (cw - ms2.x) / 2.0f, card.y + 292 }, 20, 1, HP_TEXT_DIM);
        if (sel) {
            Rectangle b = { card.x + cw / 2 - 55.0f, card.y + ch - 55.0f, 110, 36 };
            DrawRectangleRounded(b, 0.5f, 8, acc[i]);
            Vector2 sl = MeasureTextEx(bodyFont, "CHOSEN", 18, 1);
            DrawTextEx(bodyFont, "CHOSEN", { b.x + (b.width - sl.x) / 2, b.y + (b.height - sl.y) / 2 }, 18, 1, HP_DARK_BG);
        }
    }

    const char* inst = "Click to select   |   ENTER to confirm";
    Vector2 is3 = MeasureTextEx(bodyFont, inst, 24, 1);
    DrawTextEx(bodyFont, inst, { (SCREEN_W - is3.x) / 2.0f, SCREEN_H - 55.0f }, 24, 1, HP_TEXT_DIM);
}

void Game::drawPlayerNames() {
    const char* t = "REGISTER WIZARDS";
    Vector2 ts = MeasureTextEx(wizFont, t, 62, 2);
    DrawTextEx(wizFont, t, { (SCREEN_W - ts.x) / 2.0f, 75 }, 62, 2, HP_GOLD);
    struct Field { const char* label; string& val; bool active; House h; };
    Field fs[2] = {
        { "Player 1 (White)", player1, enteringP1,  house1 },
        { "Player 2 (Black)", player2, !enteringP1, house2 }
    };

    int fw = 700, fh = 60, fx = (SCREEN_W - fw) / 2;
    for (int i = 0; i < 2; i++) {
        int fy = 260 + i * 200;
        Color hc = (fs[i].h != HOUSE_NONE) ? houseColor(fs[i].h) : HP_TEXT_DIM;
        const char* hn = (fs[i].h != HOUSE_NONE) ? houseName(fs[i].h) : "No House";
        Rectangle badge = { (float)(fx - 170), (float)fy, 150, 40 };
        roundBox(badge, alpha(hc, 0.2f), hc, 0.4f);
        Vector2 bns = MeasureTextEx(bodyFont, hn, 20, 1);
        DrawTextEx(bodyFont, hn, { badge.x + (badge.width - bns.x) / 2, badge.y + 10 }, 20, 1, hc);
        DrawTextEx(bodyFont, fs[i].label, { (float)fx, (float)(fy - 36) }, 26, 1,
            fs[i].active ? HP_GOLD : HP_TEXT_DIM);
        Rectangle box = { (float)fx, (float)fy, (float)fw, (float)fh };
        roundBox(box, alpha(HP_PANEL_BG, 0.9f), fs[i].active ? HP_GOLD : alpha(HP_BORDER, 0.5f), 0.2f);

        string disp = fs[i].val;
        if (fs[i].active and (int)(animTimer * 2) % 2 == 0) disp += "|";
        DrawTextEx(wizFont, disp.c_str(), { (float)fx + 14, (float)fy + 12 }, 30, 1, WHITE);
    }

    const char* ins = "Type name -> ENTER to confirm";
    Vector2 is4 = MeasureTextEx(bodyFont, ins, 26, 1);
    DrawTextEx(bodyFont, ins, { (SCREEN_W - is4.x) / 2.0f, SCREEN_H - 75.0f }, 26, 1, HP_TEXT_DIM);
}
void Game::drawPlaying() {
    drawBoard();
    drawHighlights();
    drawEffect();
    drawRightPanel();
    if (isReplaying) drawReplay();   
    if (msgTimer > 0 and statusMsg.size()) {
        float a = (msgTimer > 0.5f) ? 1.0f : msgTimer / 0.5f;
        Vector2 ms = MeasureTextEx(wizFont, statusMsg.c_str(), 36, 1);
        float bx = (SCREEN_W - ms.x - 60) / 2.0f;
        roundBox({ bx,20,ms.x + 60,52 }, alpha(HP_PANEL_BG, 0.95f * a), alpha(HP_GOLD, a), 0.4f);
        DrawTextEx(wizFont, statusMsg.c_str(), { bx + 14, 26 }, 36, 1, alpha(HP_GOLD, a));
    }
}

void Game::drawBoard() {
    float p = 0.5f + 0.5f * sinf(animTimer * 1.4f);
    Rectangle fr = { (float)(BOARD_X - 8),(float)(BOARD_Y - 8),(float)(BOARD_SIZE + 16),(float)(BOARD_SIZE + 16) };
    DrawRectangleRounded(fr, 0.02f, 4, alpha(HP_GOLD, 0.15f + 0.07f * p));
    DrawRectangleRoundedLines(fr, 0.02f, 4, alpha(HP_GOLD, 0.5f + 0.2f * p));
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int x = BOARD_X + c * TILE_SIZE;
            int y = BOARD_Y + r * TILE_SIZE;
            Color sq = ((r + c) % 2 == 0) ? HP_WHITE_SQ : HP_BLACK_SQ;
            if (selected.row == r and selected.col == c)
                sq = lerp(sq, { 220,200,40,255 }, 0.45f);
            DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, sq);

            if (board.freezeTurns[r][c] > 0) {
                DrawRectangle(x, y, TILE_SIZE, TILE_SIZE,
                    alpha(HP_SPELL_BLUE, 0.3f + 0.2f * sinf(animTimer * 3)));
                DrawLine(x, y, x + 14, y + 14, alpha(WHITE, 0.7f));
                DrawLine(x + TILE_SIZE, y, x + TILE_SIZE - 14, y + 14, alpha(WHITE, 0.7f));
            }
            if (board.freezeTurns[r][c] == -1)
                DrawRectangle(x, y, TILE_SIZE, TILE_SIZE,
                    alpha(HP_GOLD, 0.20f + 0.12f * sinf(animTimer * 2)));
            if (board.promotionPending and board.promotionPos.row == r and board.promotionPos.col == c)
                DrawRectangle(x, y, TILE_SIZE, TILE_SIZE,
                    alpha(HP_GOLD, 0.30f + 0.15f * sinf(animTimer * 4)));
            if (board.grid[r][c]) {
                if (board.polyjuiceActive and board.polyjuice.id > 0) {
                    Color tint = (board.grid[r][c]->getColor() == CWHITE) ? WHITE : alpha(WHITE, 0.4f);
                    DrawTextureEx(board.polyjuice, { (float)x,(float)y }, 0,
                        (float)TILE_SIZE / board.polyjuice.width, tint);
                }
                else {
                    board.grid[r][c]->draw(TILE_SIZE, BOARD_X, BOARD_Y);
                }
            }
        }
    }
    for (int i = 0; i < 8; i++) {
        char rank[3], fil[3];
        snprintf(rank, 3, "%d", 8 - i);
        snprintf(fil, 3, "%c", 'a' + i);
        DrawTextEx(bodyFont, rank, { (float)(BOARD_X - 28), (float)(BOARD_Y + i * TILE_SIZE + TILE_SIZE / 2 - 12) }, 22, 1, HP_TEXT_DIM);
        DrawTextEx(bodyFont, fil, { (float)(BOARD_X + i * TILE_SIZE + TILE_SIZE / 2 - 7), (float)(BOARD_Y + BOARD_SIZE + 6) }, 22, 1, HP_TEXT_DIM);
    }
    if (spellMode) {
        DrawRectangle(BOARD_X, BOARD_Y, BOARD_SIZE, BOARD_SIZE,
            alpha(HP_SPELL_BLUE, 0.10f + 0.07f * sinf(animTimer * 5)));
        const char* aim = "Select target   |   ESC to cancel";
        Vector2 as = MeasureTextEx(bodyFont, aim, 22, 1);
        DrawRectangle(BOARD_X, BOARD_Y + BOARD_SIZE + 30, BOARD_SIZE, 36, alpha(HP_PANEL_BG, 0.88f));
        DrawTextEx(bodyFont, aim, { BOARD_X + (BOARD_SIZE - as.x) / 2.0f, (float)(BOARD_Y + BOARD_SIZE + 36) }, 22, 1, HP_SPELL_BLUE);
    }
}
void Game::drawHighlights() {
    if (selected.row == -1 or spellMode) return;
    Piece* p = board.getPiece(selected);
    if (!p) return;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            Position d = { r,c };
            if (!p->isLegal(d)) continue;
            Piece* cap = board.grid[r][c];
            Position old = p->getPos();
            board.grid[r][c] = p;
            board.grid[selected.row][selected.col] = nullptr;
            p->setPos(d);
            bool safe = !board.isInCheck(board.turn);
            board.grid[selected.row][selected.col] = p;
            board.grid[r][c] = cap;
            p->setPos(old);

            if (!safe) continue;

            int cx = BOARD_X + c * TILE_SIZE + TILE_SIZE / 2;
            int cy = BOARD_Y + r * TILE_SIZE + TILE_SIZE / 2;

            if (cap) {
                DrawCircleLines(cx, cy, TILE_SIZE / 2 - 4, HP_CAPTURE_DOT);
                DrawCircleLines(cx, cy, TILE_SIZE / 2 - 7, alpha(HP_CAPTURE_DOT, 0.5f));
            }
            else {
                DrawCircle(cx, cy, (int)(13 * (0.7f + 0.3f * sinf(animTimer * 3.5f))), HP_MOVE_DOT);
            }
        }
    }
}
void Game::drawEffect() {
    if (effect.type == E_NONE) return;
    float a = effect.timer / 0.25f;
    Color c;
    if (effect.type == GREEN_FLASH) c = alpha({ 80,230,80,255 }, a * 0.55f);
    else if (effect.type == RED_FLASH)   c = alpha(HP_SPELL_RED, a * 0.55f);
    else if (effect.type == BLUE_FLASH)  c = alpha(HP_SPELL_BLUE, a * 0.55f);
    else                                 c = alpha(HP_GOLD, a * 0.55f);
    DrawRectangle(BOARD_X + effect.pos.col * TILE_SIZE,
        BOARD_Y + effect.pos.row * TILE_SIZE,
        TILE_SIZE, TILE_SIZE, c);
}
void Game::panelDivider(int px, int pw, int& cy) {
    cy += 5;
    DrawLine(px + 20, cy, px + pw - 20, cy, alpha(HP_GOLD, 0.20f));
    DrawRectanglePro({ (float)(px + pw / 2),(float)cy,8,8 }, { 4,4 }, 45, alpha(HP_GOLD, 0.40f));
    cy += 12;
}

void Game::panelHeader(int px, int pw, int& cy) {
    DrawRectangle(px, cy, pw, 54, alpha({ 22,14,8,255 }, 0.85f));
    DrawLine(px, cy + 54, px + pw, cy + 54, alpha(HP_GOLD, 0.55f));
    DrawRectanglePro({ (float)(px + 16),(float)(cy + 27),9,9 }, { 4.5f,4.5f }, 45, alpha(HP_GOLD, 0.55f));
    DrawRectanglePro({ (float)(px + pw - 16),(float)(cy + 27),9,9 }, { 4.5f,4.5f }, 45, alpha(HP_GOLD, 0.55f));
    const char* hdr = "WIZARD CHESS";
    Vector2 hs = MeasureTextEx(wizFont, hdr, 28, 2);
    DrawTextEx(wizFont, hdr, { px + (pw - hs.x) / 2.0f, (float)(cy + 13) }, 28, 2, HP_GOLD);
    cy += 62;
}

void Game::panelTurn(int px, int pw, int& cy) {
    bool wt = (board.turn == CWHITE);
    Color tc = wt ? houseColor(house1) : houseColor(house2);
    if (tc.r == 0 and tc.g == 0 and tc.b == 0) tc = wt ? WHITE : HP_TEXT_DIM;
    float pulse = 0.5f + 0.5f * sinf(animTimer * 2.2f);

    Rectangle banner = { (float)(px + 12),(float)cy,(float)(pw - 24),72 };
    DrawRectangleRounded(banner, 0.14f, 8, alpha({ 20,13,36,255 }, 0.96f));
    DrawRectangleRounded({ banner.x,banner.y,5,banner.height }, 0.8f, 4, tc);
    DrawRectangleRounded(banner, 0.14f, 8, alpha(tc, 0.05f + 0.04f * pulse));
    DrawRectangleRoundedLines(banner, 0.14f, 8, alpha(tc, 0.45f + 0.2f * pulse));
    DrawTextEx(bodyFont, "CURRENT TURN", { banner.x + 16, banner.y + 8 }, 14, 1, alpha(tc, 0.65f));

    const char* tn = wt ? (player1.empty() ? "WHITE" : player1.c_str())
        : (player2.empty() ? "BLACK" : player2.c_str());
    DrawTextEx(wizFont, tn, { banner.x + 16, banner.y + 28 }, 28, 1, alpha(tc, 0.25f * pulse));
    DrawTextEx(wizFont, tn, { banner.x + 14, banner.y + 26 }, 28, 1, tc);

    const char* stxt; Color sc;
    if (board.promotionPending) { stxt = "PROMOTE";   sc = HP_GOLD; }
    else if (board.polyjuiceActive) { stxt = "POLYJUICE"; sc = { 220,130,20,255 }; }
    else if (showCheck) { stxt = "CHECK!";    sc = HP_SPELL_RED; }
    else if (spellMode) { stxt = "CASTING";   sc = HP_SPELL_BLUE; }
    else { stxt = "NORMAL";    sc = HP_SPELL_GREEN; }
    Vector2 sv = MeasureTextEx(bodyFont, stxt, 16, 1);
    float sbx = banner.x + banner.width - sv.x - 22;
    Rectangle pill = { sbx - 6, banner.y + 26, sv.x + 12, 24 };
    DrawRectangleRounded(pill, 0.5f, 6, alpha(sc, 0.18f));
    DrawRectangleRoundedLines(pill, 0.5f, 6, alpha(sc, 0.7f));
    DrawTextEx(bodyFont, stxt, { sbx, banner.y + 28 }, 16, 1, sc);
    cy += 80;
}

void Game::panelPlayers(int px, int pw, int& cy) {
    bool wt = (board.turn == CWHITE);
    struct PC { const char* side; string& name; House h; bool active; };
    PC cards[2] = {
        { "WHITE", player1, house1,  wt },
        { "BLACK", player2, house2, !wt }
    };

    for (int i = 0; i < 2; i++) {
        Color hc = houseColor(cards[i].h);
        Rectangle card = { (float)(px + 12),(float)cy,(float)(pw - 24),60 };
        Color bg = cards[i].active ? alpha({ 30,18,52,255 }, 0.96f)
            : alpha({ 16,11,28,255 }, 0.80f);
        DrawRectangleRounded(card, 0.14f, 8, bg);
        if (cards[i].active) {
            DrawRectangleRounded({ card.x,card.y,5,card.height }, 0.8f, 4, hc);
            DrawRectangleRounded(card, 0.14f, 8, alpha(hc, 0.06f));
        }
        DrawRectangleRoundedLines(card, 0.14f, 8,
            cards[i].active ? alpha(hc, 0.55f) : alpha(HP_BORDER, 0.22f));

        DrawTextEx(bodyFont, cards[i].side, { card.x + 14, card.y + 8 }, 16, 1,
            cards[i].active ? hc : alpha(WHITE, 0.30f));
        const char* nm = cards[i].name.empty() ? (i == 0 ? "White" : "Black") : cards[i].name.c_str();
        DrawTextEx(wizFont, nm, { card.x + 14, card.y + 27 }, 22, 1,
            cards[i].active ? WHITE : alpha(WHITE, 0.40f));

        if (cards[i].h != HOUSE_NONE) {
            const char* hn = houseName(cards[i].h);
            Vector2 hns = MeasureTextEx(bodyFont, hn, 14, 1);
            Rectangle hp2 = { card.x + card.width - hns.x - 18, card.y + 20, hns.x + 12, 20 };
            DrawRectangleRounded(hp2, 0.5f, 6, alpha(hc, 0.16f));
            DrawRectangleRoundedLines(hp2, 0.5f, 6, alpha(hc, 0.45f));
            DrawTextEx(bodyFont, hn, { hp2.x + 6, hp2.y + 3 }, 14, 1, hc);
        }
        cy += 66;
    }
}

void Game::panelCaptured(int px, int pw, int& cy) {
    DrawRectangle(px + 12, cy, 3, 22, alpha(HP_GOLD, 0.7f));
    DrawTextEx(wizFont, "Captured", { (float)(px + 22),(float)cy }, 20, 1, alpha(HP_GOLD, 0.85f));
    cy += 28;

    const char* sym[] = { "P","R","N","B","Q","K" };
    struct Row { const char* lbl; vector<PieceType>& v; PieceColor col; };
    Row rows[2] = {
        { "White took", whiteCaptured, CBLACK },
        { "Black took", blackCaptured, CWHITE }
    };

    for (int i = 0; i < 2; i++) {
        Rectangle row2 = { (float)(px + 12),(float)cy,(float)(pw - 24),28 };
        DrawRectangleRounded(row2, 0.2f, 4, alpha(HP_PANEL_EDGE, 0.4f));
        Color lc = (i == 0) ? alpha(WHITE, 0.50f) : alpha({ 200,165,90,255 }, 0.65f);
        DrawTextEx(bodyFont, rows[i].lbl, { row2.x + 8, row2.y + 6 }, 15, 1, lc);

        int cnt[6] = {};
        for (PieceType pt : rows[i].v) if (pt != T_NONE) cnt[(int)pt]++;
        bool any = false;
        int dx = (int)(row2.x + 100);
        for (int j = 0; j < 6; j++) {
            if (!cnt[j]) continue;
            any = true;
            char txt[8]; snprintf(txt, 8, "%s*%d", sym[j], cnt[j]);
            Color pc = (rows[i].col == CWHITE) ? alpha(WHITE, 0.85f) : alpha({ 220,175,80,255 }, 0.85f);
            DrawTextEx(bodyFont, txt, { (float)dx,(float)(cy + 6) }, 15, 1, pc);
            dx += (int)MeasureTextEx(bodyFont, txt, 15, 1).x + 5;
        }
        if (!any) DrawTextEx(bodyFont, "-", { row2.x + 100,(float)(cy + 6) }, 15, 1, alpha(HP_TEXT_DIM, 0.45f));
        cy += 32;
    }
}

void Game::panelSpells(int px, int pw, int& cy) {
    DrawRectangle(px + 12, cy, 3, 22, alpha(HP_GOLD, 0.7f));
    DrawTextEx(wizFont, "Spell Book", { (float)(px + 22),(float)cy }, 20, 1, alpha(HP_GOLD, 0.85f));
    cy += 30;

    PieceColor ct = board.turn;
    struct SD { const char* name; const char* key; const char* desc; Color col; SpellMode mode; };
    SD spells[] = {
        { "Glacius",   "[F]", "Freeze enemy",    HP_SPELL_BLUE,      SPELL_FREEZE    },
        { "Polyjuice", "[P]", "Hide all pieces", { 200,120,20,255 }, SPELL_POLYJUICE },
        { "Expecto",   "[E]", "Shield piece",    { 180,220,255,255 },SPELL_EXPECTO   },
        { "Wingardium","[W]", "Extra move",      HP_GOLD,            SPELL_WINGARDIUM}
    };

    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < 4; i++) {
        int ch = charges(spells[i].mode, ct);
        bool noC = (ch <= 0);
        bool isAct = (spellMode and activeSpell == spells[i].mode);
        Color bc = noC ? alpha(spells[i].col, 0.18f) : spells[i].col;

        Rectangle btn = { (float)(px + 12),(float)cy,(float)(pw - 24),50 };
        bool hov = CheckCollisionPointRec(mouse, btn);
        Color bg = isAct ? alpha(bc, 0.26f)
            : hov ? alpha({ 30,22,52,255 }, 0.95f)
            : alpha({ 16,11,30,255 }, 0.85f);
        DrawRectangleRounded(btn, 0.2f, 8, bg);
        DrawRectangleRounded({ btn.x,btn.y,4,btn.height }, 0.5f, 4, alpha(bc, noC ? 0.12f : 0.75f));
        DrawRectangleRoundedLines(btn, 0.2f, 8,
            isAct ? bc : (hov ? alpha(bc, 0.55f) : alpha(bc, 0.20f)));
        if (isAct)
            DrawRectangleRounded(btn, 0.2f, 8, alpha(bc, 0.09f * (0.4f + 0.6f * sinf(animTimer * 5))));
        Rectangle kb = { btn.x + 10, btn.y + (50 - 26) / 2.0f, 34, 26 };
        DrawRectangleRounded(kb, 0.35f, 4, alpha(bc, noC ? 0.08f : 0.18f));
        DrawTextEx(bodyFont, spells[i].key, { kb.x + 4, kb.y + 4 }, 15, 1, alpha(bc, noC ? 0.3f : 0.9f));

        DrawTextEx(wizFont, spells[i].name, { btn.x + 52, btn.y + 6 }, 20, 1, noC ? alpha(WHITE, 0.22f) : WHITE);
        DrawTextEx(bodyFont, spells[i].desc, { btn.x + 52, btn.y + 28 }, 13, 1, alpha(bc, noC ? 0.18f : 0.50f));
        for (int d = 0; d < 3; d++) {
            bool filled = (2 - d) < ch;
            int rx = (int)(btn.x + btn.width - 12 - d * 16);
            int ry = (int)(btn.y + btn.height / 2);
            DrawCircle(rx, ry, 5, filled ? bc : alpha(bc, 0.14f));
            if (filled) DrawCircleLines(rx, ry, 6, alpha(bc, 0.35f));
        }
        cy += 56;
    }
}

void Game::panelFooter(int px, int pw, int& cy) {
    int remaining = BOARD_Y + BOARD_SIZE - cy;
    DrawRectangle(px, cy, pw, remaining, alpha({ 10,7,20,255 }, 0.88f));
    DrawLine(px, cy, px + pw, cy, alpha(HP_GOLD, 0.22f));
    cy += 8;
    Rectangle replayBtn = { (float)(px + 12), (float)cy, (float)(pw - 24), 36 };
    Vector2 mouse = GetMousePosition();
    bool replayHov = CheckCollisionPointRec(mouse, replayBtn);
    Color replayCol = isReplaying ? HP_SPELL_GREEN : HP_SPELL_BLUE;

    DrawRectangleRounded(replayBtn, 0.3f, 8,
        isReplaying ? alpha(HP_SPELL_GREEN, 0.22f) :
        replayHov ? alpha(HP_SPELL_BLUE, 0.18f) :
        alpha(HP_PANEL_EDGE, 0.70f));
    DrawRectangleRoundedLines(replayBtn, 0.3f, 8,
        replayHov or isReplaying ? replayCol : alpha(replayCol, 0.35f));
    const char* replayLabel = isReplaying ? ">> REPLAYING... (click to stop)" : "[T]  Replay Moves";
    Vector2 rls = MeasureTextEx(bodyFont, replayLabel, 16, 1);
    DrawTextEx(bodyFont, replayLabel,
        { replayBtn.x + (replayBtn.width - rls.x) / 2, replayBtn.y + 10 },
        16, 1, replayHov or isReplaying ? replayCol : alpha(WHITE, 0.65f));

    cy += 44;

    struct SC { const char* key; const char* desc; };
    SC sc[] = { {"S","Save"},{"L","Load"},{"Z","Undo"},{"Y","Redo"},{"ESC","Cancel"} };
    int cols = 5;
    int btnW = (pw - 24) / cols;

    for (int i = 0; i < cols; i++) {
        int fx = px + 12 + i * btnW;
        Rectangle pill = { (float)fx,(float)cy,(float)(btnW - 5),32 };
        DrawRectangleRounded(pill, 0.4f, 6, alpha(HP_PANEL_EDGE, 0.55f));
        DrawRectangleRoundedLines(pill, 0.4f, 6, alpha(HP_GOLD, 0.15f));
        char full[8]; snprintf(full, sizeof(full), "[%s]", sc[i].key);
        Vector2 ks = MeasureTextEx(bodyFont, full, 13, 1);
        DrawTextEx(bodyFont, full, { pill.x + (pill.width - ks.x) / 2, pill.y + 3 }, 13, 1, alpha(HP_GOLD, 0.75f));
        Vector2 ds = MeasureTextEx(bodyFont, sc[i].desc, 11, 1);
        DrawTextEx(bodyFont, sc[i].desc, { pill.x + (pill.width - ds.x) / 2, pill.y + 18 }, 11, 1, HP_TEXT_DIM);
    }
}

void Game::drawRightPanel() {
    int px = RPANEL_X, pw = RPANEL_W, py = BOARD_Y;
    DrawRectangleRounded({ (float)(px + 4),(float)(py + 5),(float)pw,(float)BOARD_SIZE },
        0.04f, 8, alpha(BLACK, 0.40f));
    DrawRectangleRounded({ (float)px,(float)py,(float)pw,(float)BOARD_SIZE },
        0.04f, 8, alpha({ 13,9,26,255 }, 0.97f));
    DrawRectangleRoundedLines({ (float)px,(float)py,(float)pw,(float)BOARD_SIZE },
        0.04f, 8, alpha(HP_GOLD, 0.28f));
    DrawRectangleRoundedLines({ (float)(px + 4),(float)(py + 4),(float)(pw - 8),(float)(BOARD_SIZE - 8) },
        0.04f, 8, alpha(HP_GOLD, 0.09f));

    int cy = py;
    panelHeader(px, pw, cy);
    panelTurn(px, pw, cy);
    panelDivider(px, pw, cy);
    panelPlayers(px, pw, cy);
    panelDivider(px, pw, cy);
    panelCaptured(px, pw, cy);
    panelDivider(px, pw, cy);
    panelSpells(px, pw, cy);
    panelFooter(px, pw, cy);
}
void Game::drawPromotion() {
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, alpha(HP_DARK_BG, 0.72f));

    PieceColor col = board.promotionColor;
    const char* who = (col == CWHITE) ? (player1.empty() ? "White" : player1.c_str())
        : (player2.empty() ? "Black" : player2.c_str());
    float cardW = 780, cardH = 340;
    Rectangle card = { (SCREEN_W - cardW) / 2, (SCREEN_H - cardH) / 2, cardW, cardH };
    DrawRectangleRounded(card, 0.08f, 8, alpha(HP_PANEL_BG, 0.97f));
    DrawRectangleRoundedLines(card, 0.08f, 8, HP_GOLD);
    DrawRectangleRounded({ card.x,card.y,card.width,52 }, 0.08f, 8, alpha({ 28,18,8,255 }, 0.9f));
    DrawLine((int)card.x, (int)(card.y + 52), (int)(card.x + card.width), (int)(card.y + 52), alpha(HP_GOLD, 0.45f));
    char title[64]; snprintf(title, sizeof(title), "  %s - Choose Promotion", who);
    Vector2 ts = MeasureTextEx(wizFont, title, 30, 1);
    DrawTextEx(wizFont, title, { card.x + (card.width - ts.x) / 2, card.y + 12 }, 30, 1, HP_GOLD);

    const char* sub = "Click a piece to promote your Pawn";
    Vector2 ss2 = MeasureTextEx(bodyFont, sub, 22, 1);
    DrawTextEx(bodyFont, sub, { card.x + (card.width - ss2.x) / 2, card.y + 66 }, 22, 1, HP_TEXT_DIM);
    struct Choice { const char* name; PieceType type; Texture2D& tex; Color glow; };
    Choice choices[4] = {
        { "Queen",  QUEEN,  (col == CWHITE) ? board.wq : board.bq, HP_GOLD         },
        { "Rook",   ROOK,   (col == CWHITE) ? board.wr : board.br, HP_SPELL_BLUE   },
        { "Bishop", BISHOP, (col == CWHITE) ? board.wb : board.bb, HP_SPELL_GREEN  },
        { "Knight", KNIGHT, (col == CWHITE) ? board.wn : board.bn, {200,130,255,255} }
    };

    float btnW = 150, btnH = 175, spacing = 20;
    float totalW = 4 * btnW + 3 * spacing;
    float startX = card.x + (card.width - totalW) / 2;
    float btnY = card.y + 100;
    Vector2 mouse = GetMousePosition();
    float pulse = 0.5f + 0.5f * sinf(animTimer * 3.0f);

    for (int i = 0; i < 4; i++) {
        Rectangle btn = { startX + i * (btnW + spacing), btnY, btnW, btnH };
        bool hov = CheckCollisionPointRec(mouse, btn);

        DrawRectangleRounded(btn, 0.12f, 8, hov ? alpha(choices[i].glow, 0.20f) : alpha(HP_PANEL_EDGE, 0.8f));
        DrawRectangleRoundedLines(btn, 0.12f, 8, hov ? choices[i].glow : alpha(choices[i].glow, 0.38f));
        if (hov) DrawRectangleRounded(btn, 0.12f, 8, alpha(choices[i].glow, 0.09f * pulse));

        if (choices[i].tex.id > 0) {
            float sc2 = 85.0f / choices[i].tex.width;
            DrawTextureEx(choices[i].tex, { btn.x + (btnW - 85) / 2, btn.y + 12 }, 0, sc2, WHITE);
        }

        Vector2 ns = MeasureTextEx(wizFont, choices[i].name, 22, 1);
        DrawTextEx(wizFont, choices[i].name,
            { btn.x + (btnW - ns.x) / 2, btn.y + btnH - 36 }, 22, 1,
            hov ? choices[i].glow : WHITE);
    }
}
void Game::drawGameOver() {
    Rectangle card = { (SCREEN_W - 680.0f) / 2, (SCREEN_H - 460.0f) / 2, 680, 460 };
    DrawRectangleRounded(card, 0.07f, 8, alpha(HP_PANEL_BG, 0.96f));
    DrawRectangleRoundedLines(card, 0.07f, 8, HP_GOLD);

    const char* res = (gameResult == CHECKMATE) ? "CHECKMATE!" : "STALEMATE!";
    Color rc = (gameResult == CHECKMATE) ? HP_GOLD : HP_TEXT_DIM;
    float p = 0.5f + 0.5f * sinf(animTimer * 2);
    Vector2 rs = MeasureTextEx(wizFont, res, 70, 2);
    DrawTextEx(wizFont, res, { card.x + (card.width - rs.x) / 2, card.y + 44 }, 70, 2, alpha(rc, 0.22f * p));
    DrawTextEx(wizFont, res, { card.x + (card.width - rs.x) / 2, card.y + 42 }, 70, 2, rc);

    if (gameResult == CHECKMATE) {
        const char* win = (board.turn == CBLACK)
            ? (player1.empty() ? "White Wins!" : (player1 + " Wins!").c_str())
            : (player2.empty() ? "Black Wins!" : (player2 + " Wins!").c_str());
        Vector2 ws = MeasureTextEx(wizFont, win, 42, 1);
        DrawTextEx(wizFont, win, { card.x + (card.width - ws.x) / 2, card.y + 145 }, 42, 1, WHITE);
    }

    DrawLine((int)(card.x + 50), (int)(card.y + 215), (int)(card.x + card.width - 50), (int)(card.y + 215), alpha(HP_GOLD, 0.28f));
    char sw[64], sb[64];
    snprintf(sw, 64, "White captured: %d", (int)whiteCaptured.size());
    snprintf(sb, 64, "Black captured: %d", (int)blackCaptured.size());
    DrawTextEx(bodyFont, sw, { card.x + 50, card.y + 230 }, 24, 1, alpha(WHITE, 0.7f));
    DrawTextEx(bodyFont, sb, { card.x + 50, card.y + 260 }, 24, 1, alpha(WHITE, 0.5f));

    struct Btn { const char* l; float ox; Color c; };
    Btn btns[] = {
        { "Play Again [R]", 40,               HP_GOLD       },
        { "Replay [T]",     card.width / 2 - 100, HP_SPELL_GREEN },
        { "Main Menu [M]",  card.width - 280, HP_SPELL_BLUE }
    };
    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < 3; i++) {
        Rectangle r = { card.x + btns[i].ox, card.y + 340, 200, 58 };
        bool hov = CheckCollisionPointRec(mouse, r);
        roundBox(r, alpha(HP_PANEL_EDGE, 0.9f), hov ? btns[i].c : alpha(btns[i].c, 0.45f), 0.3f);
        if (hov) DrawRectangleRounded(r, 0.3f, 8, alpha(btns[i].c, 0.12f));
        Vector2 ls = MeasureTextEx(wizFont, btns[i].l, 22, 1);
        DrawTextEx(wizFont, btns[i].l, { r.x + (r.width - ls.x) / 2, r.y + (r.height - 22) / 2 }, 22, 1, hov ? btns[i].c : WHITE);
    }
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Rectangle rR = { card.x + 40,               card.y + 340, 200, 58 };
        Rectangle rT = { card.x + card.width / 2 - 100, card.y + 340, 200, 58 };
        Rectangle rM = { card.x + card.width - 280,  card.y + 340, 200, 58 };
        if (CheckCollisionPointRec(mouse, rT)) startReplay();
    }
}
void Game::handleMenu() {
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;
    Vector2 mouse = GetMousePosition();
    Rectangle twoPlayer = { (SCREEN_W - 360.0f) / 2, SCREEN_H * 0.44f - 30, 360, 60 };
    Rectangle vsAIBtn = { (SCREEN_W - 360.0f) / 2, SCREEN_H * 0.54f - 30, 360, 60 };
    Rectangle load = { (SCREEN_W - 360.0f) / 2, SCREEN_H * 0.64f - 30, 360, 60 };
    Rectangle exitr = { (SCREEN_W - 360.0f) / 2, SCREEN_H * 0.74f - 30, 360, 60 };
    if (CheckCollisionPointRec(mouse, twoPlayer)) { vsAI = false; state = HOUSE_SELECT; }
    if (CheckCollisionPointRec(mouse, vsAIBtn)) { vsAI = true;  state = HOUSE_SELECT; }
    if (CheckCollisionPointRec(mouse, load))  loadGame();
    if (CheckCollisionPointRec(mouse, exitr)) CloseWindow();
}

void Game::handleHouseSelect() {
    Vector2 mouse = GetMousePosition();
    int cw = 330, ch = 450, gap = 40;
    int sx = (SCREEN_W - (4 * cw + 3 * gap)) / 2;
    int startY = (SCREEN_H - ch) / 2 + 20;
    bool isP1 = (houseStep == 0);

    for (int i = 0; i < 4; i++) {
        Rectangle card = { (float)(sx + i * (cw + gap)), (float)startY, (float)cw, (float)ch };
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) and CheckCollisionPointRec(mouse, card)) {
            if (isP1) house1 = (House)i;
            else      house2 = (House)i;
        }
    }

    if (IsKeyPressed(KEY_ENTER)) {
        if (isP1 and house1 != HOUSE_NONE) {
            if (vsAI) { house2 = SLYTHERIN; houseStep = 1; state = PLAYER_NAMES; }
            else       houseStep = 1;
        }
        else if (!isP1 and house2 != HOUSE_NONE) state = PLAYER_NAMES;
    }
}

void Game::handlePlayerNames() {
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 and key < 128) {
            if (enteringP1 and player1.size() < 18) player1 += (char)key;
            if (!enteringP1 and player2.size() < 18) player2 += (char)key;
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (enteringP1 and !player1.empty()) player1.pop_back();
        if (!enteringP1 and !player2.empty()) player2.pop_back();
    }

    if (IsKeyPressed(KEY_ENTER)) {
        if (enteringP1 and !player1.empty()) {
            if (vsAI) {
                player2 = "AI Wizard";
                house2 = SLYTHERIN;
                board.init();
                whiteCaptured.clear();
                blackCaptured.clear();
                wFreeze = bFreeze = 2; wExpecto = bExpecto = 2; wWing = bWing = wPoly = bPoly = 1;
                state = PLAYING;
            }
            else {
                enteringP1 = false;
            }
        }
        else if (!enteringP1 and !player2.empty()) {
            board.init();
            whiteCaptured.clear();
            blackCaptured.clear();
            wFreeze = bFreeze = 2; wExpecto = bExpecto = 2; wWing = bWing = wPoly = bPoly = 1;
            state = PLAYING;
        }
    }
}
void Game::handlePromotion() {
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;
    Vector2 mouse = GetMousePosition();

    float btnW = 150, btnH = 175, spacing = 20;
    float totalW = 4 * btnW + 3 * spacing;
    float cardW = 780, cardH = 340;
    float cardX = (SCREEN_W - cardW) / 2;
    float cardY = (SCREEN_H - cardH) / 2;
    float startX = cardX + (cardW - totalW) / 2;
    float btnY = cardY + 100;

    PieceType choices[4] = { QUEEN, ROOK, BISHOP, KNIGHT };
    for (int i = 0; i < 4; i++) {
        Rectangle btn = { startX + i * (btnW + spacing), btnY, btnW, btnH };
        if (CheckCollisionPointRec(mouse, btn)) {
            board.applyPromotion(choices[i]);
            state = PLAYING;
            return;
        }
    }
}

void Game::handlePlaying() {
    if (vsAI and board.turn == CBLACK) return;
    if (isReplaying) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) or IsKeyPressed(KEY_T)) {
            isReplaying = false;
            board.init();
            whiteCaptured.clear();
            blackCaptured.clear();
            wFreeze = bFreeze = 2;
            wExpecto = bExpecto = 2;
            wWing = bWing = wPoly = bPoly = 1;
            for (int i = 0; i < moveCount; i++)
                board.move(moveHistory[i].from, moveHistory[i].to);
            setMsg("Replay stopped.");
        }
        return;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (spellMode) { spellMode = false; activeSpell = SPELL_NONE; }
        else selected = { -1,-1 };
        return;
    }

    if (IsKeyPressed(KEY_Z)) doUndo();
    if (IsKeyPressed(KEY_Y)) doRedo();
    if (IsKeyPressed(KEY_S)) saveGame();
    if (IsKeyPressed(KEY_L)) loadGame();
    if (IsKeyPressed(KEY_T)) { if (moveCount > 0) startReplay(); else setMsg("No moves yet!"); }
    auto trySpell = [&](SpellMode sm, const char* msg) {
        if (charges(sm, board.turn) > 0) { spellMode = true; activeSpell = sm; setMsg(msg); }
        else setMsg("No charges!");
        };
    if (IsKeyPressed(KEY_F)) trySpell(SPELL_FREEZE, "Glacius! Freeze an enemy piece.");
    if (IsKeyPressed(KEY_P)) trySpell(SPELL_POLYJUICE, "Polyjuice! All pieces disguised.");
    if (IsKeyPressed(KEY_E)) trySpell(SPELL_EXPECTO, "Expecto Patronum! Shield a piece.");
    if (IsKeyPressed(KEY_W)) trySpell(SPELL_WINGARDIUM, "Wingardium! Move a piece again.");

    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) { selected = { -1,-1 }; return; }
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;

    Vector2 mouse = GetMousePosition();

    Rectangle replayBtn = { (float)(RPANEL_X + 12), (float)(BOARD_Y + BOARD_SIZE - 88),
                            (float)(RPANEL_W - 24), 36 };
    if (CheckCollisionPointRec(mouse, replayBtn)) {
        if (moveCount > 0) startReplay();
        else setMsg("No moves to replay yet!");
        return;
    }
    int col = (int)((mouse.x - BOARD_X) / TILE_SIZE);
    int row = (int)((mouse.y - BOARD_Y) / TILE_SIZE);
    if (row < 0 or row >= 8 or col < 0 or col >= 8) 
    { selected = { -1,-1 }; return; }
    Position clicked = { row, col };
    if (spellMode) {
        Piece* tgt = board.getPiece(clicked);

        if (activeSpell == SPELL_FREEZE) {
            if (tgt and tgt->getColor() != board.turn) {
                if (board.castFreeze(clicked)) {
                    spend(SPELL_FREEZE, board.turn == CWHITE ? CBLACK : CWHITE);
                    effect = { BLUE_FLASH, 0.5f, clicked };
                    setMsg("Glacius! Frozen for 2 turns.");
                }
            }
            else setMsg("Target an enemy piece!");
        }
        else if (activeSpell == SPELL_POLYJUICE) {
            spend(SPELL_POLYJUICE, board.turn);
            board.polyjuiceActive = true;
            board.turn = (board.turn == CWHITE) ? CBLACK : CWHITE;
            setMsg("Polyjuice! Lasts 3 turns.");
            effect = { GOLD_FLASH, 0.6f, clicked };
        }
        else if (activeSpell == SPELL_EXPECTO) {
            if (tgt and tgt->getColor() == board.turn) {
                board.freezeTurns[row][col] = -1;
                spend(SPELL_EXPECTO, board.turn);
                board.turn = (board.turn == CWHITE) ? CBLACK : CWHITE;
                effect = { GOLD_FLASH, 0.5f, clicked };
                setMsg("Expecto Patronum! Piece shielded.");
            }
            else setMsg("Choose your own piece!");
        }
        else if (activeSpell == SPELL_WINGARDIUM) {
            if (tgt and tgt->getColor() == board.turn) {
                selected = clicked;
                spend(SPELL_WINGARDIUM, board.turn);
                setMsg("Wingardium! Now move it.");
            }
            else setMsg("Choose your own piece!");
        }

        spellMode = false;
        activeSpell = SPELL_NONE;
        return;
    }

    if (selected.row == -1) {
        Piece* p = board.getPiece(clicked);
        if (p and p->getColor() == board.turn) selected = clicked;
    }
    else {
        Piece* p = board.getPiece(selected);
        if (!p) { selected = { -1,-1 }; return; }

        Piece* dest = board.getPiece(clicked);
        PieceType captType = dest ? dest->getType() : T_NONE;

        pushUndo();
        bool moved = board.move(selected, clicked);

        if (moved) {
            if (moveCount < MAX_MOVES) {
                moveHistory[moveCount].from = selected;
                moveHistory[moveCount].to = clicked;
                moveCount++;
            }

            if (captType != T_NONE) {
                effect = { GREEN_FLASH, 0.5f, clicked };
                if (board.turn == CBLACK) whiteCaptured.push_back(captType);
                else                      blackCaptured.push_back(captType);
            }
            else {
                effect = { GREEN_FLASH, 0.25f, clicked };
            }
            if (board.polyjuiceActive) {
                static int polyTurns = 0;
                polyTurns++;
                if (polyTurns >= 6) {
                    board.polyjuiceActive = false;
                    polyTurns = 0;
                    setMsg("Polyjuice wears off!");
                }
            }
        }
        else {
            board.freeSnapshot(undoStack.data[undoStack.top]);
            undoStack.top--;
            effect = { RED_FLASH, 0.25f, clicked };
            Piece* q = board.getPiece(clicked);
            if (q and q->getColor() == board.turn) { selected = clicked; return; }
        }
        selected = { -1,-1 };
    }
}