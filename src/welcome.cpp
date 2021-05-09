#include "game.h"
#include "anim.h"

#include <algorithm> 

WelcomeAnimation::WelcomeAnimation(Game *game) : game(game) {}

WelcomeAnimation::~WelcomeAnimation() {
    UnloadTexture(texture);
}

void WelcomeAnimation::Init() {
    texture = LoadTexture("resources/welcome.png");
}

void WelcomeAnimation::Update() {
    if (game->framesCounter >= DURATION + ANIMATION / 2) {
        game->SetScene(MAIN);
    }
}

void WelcomeAnimation::Draw() {
    DrawTexture(texture, 0, 0, Fade(WHITE, 1.0f - 1.0f * std::min(std::max(game->framesCounter, 0) - DURATION, ANIMATION) / ANIMATION));
}

bool WelcomeAnimation::ShouldRender() {
    return game->framesCounter <= DURATION + ANIMATION;

}