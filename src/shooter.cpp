#include <algorithm>
#include <string>
#include "game.h"
#include "shooter.h"
#include "input.h"

Shooter::Shooter(Game* game, ShooterPosition pos) : game(game), pos(pos) {}

Shooter::~Shooter() {

}

void Shooter::Update() {
    if (pos == DOWN && !game->IsMultiPlayer()) return;

    if (pos == UP) {
        InputController* input = game->GetInput();
        input->SetEnergy(std::min(input->GetEnergy(), 500));
    }
    
}

void Shooter::Draw() {
    if (pos == DOWN && !game->IsMultiPlayer()) return;

    int L = std::min(game->screenWidth, game->screenHeight);

    InputController* input = game->GetInput();
    float cx = pos == UP ? input->GetCursorX() : 0.5f * L;
    float startY = (pos == UP ? 0.03f : 0.97f) * L;
    float endY = (pos == UP ? 0.1f : 0.9f) * L;
    DrawLineEx((Vector2){cx - 0.1f * L, startY}, (Vector2){cx - 0.02f * L, endY}, 3, WHITE);
    DrawLineEx((Vector2){cx + 0.1f * L, startY}, (Vector2){cx + 0.02f * L, endY}, 3, WHITE); 

    if (pos == UP) {
        DrawText(std::to_string(input->GetEnergy()).c_str(), 0.02f * L, 0.02f * L, 48, WHITE);
    }
}
