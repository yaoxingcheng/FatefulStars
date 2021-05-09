#ifndef __GAME_H__
#define __GAME_H__

#include "raylib.h"

#include "physac.h"

class Planet;
class Shooter;
class InputController;
class NetworkManager;

class Game {
public:
    static const int screenWidth = 600;
    static const int screenHeight = 600;
    static constexpr float ball_radius = 10.0;
    static constexpr float ball_dense = 10.0;

    static constexpr float pull_coef = 1e-5;

    int framesCounter;
    bool gameOver;

    Game();
    ~Game();

    void InitGame(void);         // Initialize game
    void UpdateGame(void);       // Update game (one frame)
    void DrawGame(void);         // Draw game (one frame)
    void UnloadGame(void);       // Unload game
    void UpdateDrawFrame(void);  // Update and Draw (one frame)

    bool IsMultiPlayer(void);
    InputController* GetInput(void);
    Planet* GetPlanet(void);
    
private:
    Planet* planet;
    Shooter* shooter;
    Shooter* oppositeShooter;
    InputController *input;
    NetworkManager* networkManager;
};

#endif
