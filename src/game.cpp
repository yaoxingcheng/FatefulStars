#include "game.h"

#define PHYSAC_IMPLEMENTATION
#include "external/physac.h"

int main(void) {
    Game *g = new Game();
    InitWindow(g->screenWidth, g->screenHeight, "Game");
    g->InitGame();

    SetTargetFPS(60);

    // Main game loop
    // Detect window close button or ESC key
    while (!WindowShouldClose()) {
        g->UpdateGame();
        g->DrawGame();
    }

    CloseWindow();

    return 0;
}
