#include "game.h"
#include "planet.h"
#include "shooter.h"
#include "input.h"
#include "network.h"

Game::Game() {
    planet = new Planet(this);
    shooter = new Shooter(this, UP, ball_radius, ball_dense);
    oppositeShooter = new Shooter(this, DOWN, ball_radius, ball_dense);
    input = new InputController(this);
    networkManager = new NetworkManager(this);
}

Game::~Game() {
    delete planet;
    delete shooter;
    delete oppositeShooter;
    delete input;
    delete networkManager;
}

// Initialize game variables
void Game::InitGame(void) {
    framesCounter = 0;
    gameOver = false;
}

// Update game per frame
void Game::UpdateGame(void) {
    if (!gameOver) {
        input->Update();
        planet->Update();
        shooter->Update();
        oppositeShooter->Update();
    }
}

// Draw game per frame
void Game::DrawGame(void) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    if (!gameOver) {
        planet->Draw();
        shooter->Draw();
        oppositeShooter->Draw();
    }

    EndDrawing();
}

bool Game::IsMultiPlayer(void) {
    return true;
}

InputController* Game::GetInput(void) {
    return input;
}

Planet* Game::GetPlanet(void) { 
    return planet;
}