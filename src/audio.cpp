#include "audio.h"

AudioController::AudioController() {
    shoot = LoadSound("resources/fashe.mp3");
    collide = LoadSound("resources/zhuang.mp3");
    destroy = LoadSound("resources/posui.mp3");
}

AudioController::~AudioController() {
    UnloadSound(shoot);
    UnloadSound(collide);
    UnloadSound(destroy);
}

AudioController::Update() {
    shooted = collided = destroyed = 0;
}

AudioController::Shoot() {
    if (shooted) return;
    PlaySoundMulti(shoot);
    shooted = 1;
}

AudioController::Collide() {
    if (collided) return;
    PlaySoundMulti(collide);
    collided = 1;
}

AudioController::Destroy() {
    if (destroyed) return;
    PlaySoundMulti(destroy);
    destroyed = 1;
}
