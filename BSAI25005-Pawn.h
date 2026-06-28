#include "raylib.h"
#include "BSAI25005-Game.h"
#include "BSAI25005-Globals.h"

int main() {
    // 1. Initialize the Window using constants from Globals.h
    // This will open a window that fits your laptop screen perfectly.
    InitWindow(SCREEN_W, SCREEN_H, "Wizard Chess - BSAI25005");
    InitAudioDevice();
    Music music = LoadMusicStream("harry_potter.mp3");
    SetMusicVolume(music, 0.2f);    // This ensures smooth movement and consistent timing for spell effects.
    SetTargetFPS(60);
    PlayMusicStream(music);
    // 3. Create and Initialize the Game
    // This loads all piece textures (Pawns, Kings, etc.) and sets the board.
    Game wizardChess;
    wizardChess.init();

    // 4. The Main Game Loop
    // This runs continuously until you press the ESC key or click the 'X'.
    while (!WindowShouldClose()) {
        // 1. Update logic
        UpdateMusicStream(music);
        wizardChess.update();

        // 2. Draw logic (The ONLY place for these two lines)
        BeginDrawing();
        wizardChess.draw();
        EndDrawing();
    }

    // 5. Cleanup
    // Raylib will automatically unload remaining resources here.
    CloseWindow();

    return 0;
}