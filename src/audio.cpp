#include "game.h"
#include "audio.h"
#include <cstdio>

AudioController::AudioController() {
    InitAudioDevice();
    bgm = LoadSound("resources/mc.mp3");
    shoot = LoadSound("resources/fashe.mp3");
    collide = LoadSound("resources/zhuang.mp3");
    destroy = LoadSound("resources/posui.mp3");
    PlaySound(bgm);
    printf("feaifewnfoeiwfnwei\n\n\n\n\n\n\n");
}

AudioController::~AudioController() {
    UnloadSound(shoot);
    UnloadSound(collide);
    UnloadSound(destroy);
}

void AudioController::Update() {
    shooted = collided = destroyed = 0;
    if (!IsSoundPlaying(bgm)) PlaySound(bgm);
}

void AudioController::Shoot() {
    if (shooted) return;
    PlaySoundMulti(shoot);
    shooted = 1;
}

void AudioController::Collide() {
    if (collided) return;
    PlaySoundMulti(collide);
    collided = 1;
}

void AudioController::Destroy() {
    if (destroyed) return;
    PlaySoundMulti(destroy);
    destroyed = 1;
}
