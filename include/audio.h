#ifndef __AUDIO_H__
#define __AUDIO_H__

class AudioController {
public:
    AudioController();
    ~AudioController();

    void Update();
    void Shoot();
    void Collide();
    void Destroy();

private:
    bool shooted, collided, destroyed;
    Sound shoot, collide, destroy;
};

#endif