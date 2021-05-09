#include <algorithm>
#include "game.h"
#include "planet.h"

Planet::Planet(Game* game) : game(game) {
    int L = std::min(game->screenWidth, game->screenHeight);
    x_position = L / 2;
    y_position = L / 2;
    radius = L / 8;
    dense = 100;
    body = CreatePhysicsBodyCircle((Vector2){float(x_position), float(y_position)}, radius, dense);
    body->enabled = false;
    body_id = body->id;
}

Planet::~Planet() {

}

void Planet::Update() {

}

void Planet::Draw() {
    DrawCircle(x_position, y_position, radius, LIGHTGRAY);
}
