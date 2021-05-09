#ifndef __GAME_H__
#define __GAME_H__

#include "raylib.h"
#include "game_state.h"

#include "physac.h"

#include <iostream>
using namespace std;

class Planet;
class Shooter;
class InputController;
class NetworkManager;

enum Scene {
    MAIN,
    NETWORK_SETTINGS,
};

class Game {
public:
    static const int screenWidth = 800;
    static const int screenHeight = 800;
    static const int min_sides = 3;
    static const int max_sides = 6;
    static constexpr float ball_radius = 30.0;
    static constexpr float ball_dense = 10.0;
    static constexpr float shooter_radius = 35.0;
    static constexpr float shooter_dense = 10.0;
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
    

    Scene GetScene();
    void SetScene(Scene);

    void DumpState(GameState *state);
    void LoadState(const GameState *state);

private:
    Scene scene;
    Planet* planet;
    Shooter* shooter;
    Shooter* oppositeShooter;
    InputController *input;
    NetworkManager* networkManager;
};

#endif
