#include "game.h"
#include "planet.h"
#include "shooter.h"
#include "input.h"
#include "network.h"

Game::Game() {
    planet = new Planet(this);
    shooter = new Shooter(this, UP);
    oppositeShooter = new Shooter(this, DOWN);
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

    networkManager->Init();
}

// Update game per frame
void Game::UpdateGame(void) {
    if (!gameOver) {
        framesCounter ++;
        input->Update();
        networkManager->Update();
        planet->Update();
        shooter->Update();
        oppositeShooter->Update();
    }
}

// Draw game per frame
void Game::DrawGame(void) {
    BeginDrawing();
    ClearBackground(BLACK);

    if (!gameOver) {
        if (scene == MAIN) {
            planet->Draw();
            shooter->Draw();
            oppositeShooter->Draw();
            networkManager->DrawMainButton();
        } else if (scene == NETWORK_SETTINGS) {
            networkManager->DrawSettingsPanel();
        }
    }

    EndDrawing();
}

bool Game::IsMultiPlayer(void) {
    return true;
}

void Game::SetScene(Scene newScene) {
    scene = newScene;
}

Scene Game::GetScene() {
    return scene;
}

InputController* Game::GetInput(void) {
    return input;
}
