#include "audio.h"

AudioController::AudioController() {
    bgm = LoadSound("resources/mc.mp3");
    shoot = LoadSound("resources/fashe.mp3");
    collide = LoadSound("resources/zhuang.mp3");
    destroy = LoadSound("resources/posui.mp3");
    PlaySound(bgm);
}

AudioController::~AudioController() {
    UnloadSound(shoot);
    UnloadSound(collide);
    UnloadSound(destroy);
}

AudioController::Update() {
    shooted = collided = destroyed = 0;
    if (!IsSoundPlaying(bgm)) PlaySound(bgm);
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
