#include "game.h"
#include "planet.h"
#include "shooter.h"
#include "input.h"
#include "network.h"
#include "anim.h"

Game::Game() {
    scene = WELCOME;
    planet = new Planet(this);
    shooter = new Shooter(this, UP, ball_radius, ball_dense);
    oppositeShooter = new Shooter(this, DOWN, ball_radius, ball_dense);
    input = new InputController(this);
    networkManager = new NetworkManager(this);
    welcomeAnim = new WelcomeAnimation(this);
}

Game::~Game() {
    delete planet;
    delete shooter;
    delete oppositeShooter;
    delete input;
    delete networkManager;
    delete welcomeAnim;
}

// Initialize game variables
void Game::InitGame(void) {
    framesCounter = 0;
    gameOver = false;
    pause = false;
    networkManager->Init();
    welcomeAnim->Init();
    planet->Init();
    shooter->Init();
    oppositeShooter->Init();
}

// Update game per frame
void Game::UpdateGame(void) {
    if (!gameOver) {
        framesCounter ++;
        if (scene == WELCOME) {
            welcomeAnim->Update();
        } else {
            input->Update();
            networkManager->Update();
            planet->Update();
            shooter->Update();
            oppositeShooter->Update();
        }

    }
}

// Draw game per frame
void Game::DrawGame(void) {
    BeginDrawing();
    ClearBackground(BLACK);

    if (!gameOver) {
        if (scene == WELCOME) {
            welcomeAnim->Draw();
        } else if (scene == MAIN) {
            if (welcomeAnim->ShouldRender()) {
                welcomeAnim->Draw();
            }
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
    return networkManager->IsConnected();
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

Planet* Game::GetPlanet(void) { 
    return planet;
}
void Game::LoadState(const GameState *state) {
    oppositeShooter->SetCursorX(state->cursorX);
}

void Game::DumpState(GameState *state) {
    state->finished = !networkManager->IsConnected();
    state->cursorX = input->GetCursorX();
}
