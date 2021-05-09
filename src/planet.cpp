#include <algorithm>
#include "game.h"
#include "planet.h"

Planet::Planet(Game* game) : game(game) {}

Planet::~Planet() {

}

void Planet::Update() {

}

void Planet::Draw() {
    int L = std::min(game->screenWidth, game->screenHeight);
    DrawCircle(L / 2, L / 2, L / 8, LIGHTGRAY);
}
