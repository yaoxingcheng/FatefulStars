#include <algorithm>
#include "game.h"
#include "planet.h"
#include "anim.h"

Planet::Planet(Game* game) : game(game) {
    int L = std::min(game->screenWidth, game->screenHeight);
    x_position = L / 2;
    y_position = L / 2;
    radius = 110;
    dense = 100;
    body = CreatePhysicsBodyCircle((Vector2){float(x_position), float(y_position)}, radius, dense);
    body->enabled = false;
}

Planet::~Planet() {
    UnloadTexture(bg);
    for (int i = 0; i < 4; ++i) {
        UnloadTexture(balls[i]);
    }
    delete[] balls;
}

void Planet::Init() {
    bg = LoadTexture("resources/bg.png");
    balls = new Texture2D[4];
    for (int i = 0; i < 4; ++i) {
        balls[i] = LoadTexture(TextFormat("resources/ball%d.png", i));
    }
}

int Planet::GetID() {
    if (body == NULL) return -1;
    return body->id;
}

void Planet::Update() {

}

void Planet::Draw() {
    const int DURATION = WelcomeAnimation::DURATION;
    const int ANIMATION = WelcomeAnimation::ANIMATION;
    DrawTexture(bg, 0, 0, Fade(WHITE, 1.0f * std::min(game->framesCounter - DURATION - ANIMATION / 2, ANIMATION) / ANIMATION));
    DrawTexture(balls[0], 0, 0, Fade(WHITE, 1.0f * std::min(game->framesCounter - DURATION - ANIMATION / 2, ANIMATION) / ANIMATION));
}
