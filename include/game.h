#ifndef __GAME_H__
#define __GAME_H__

#include "raylib.h"

class Game {
public:
    static const int screenWidth = 800;
    static const int screenHeight = 800;

    int framesCounter;
    bool gameOver;

    Game();
    ~Game();

    void InitGame(void);         // Initialize game
    void UpdateGame(void);       // Update game (one frame)
    void DrawGame(void);         // Draw game (one frame)
    void UnloadGame(void);       // Unload game
    void UpdateDrawFrame(void);  // Update and Draw (one frame)
};

#endif
