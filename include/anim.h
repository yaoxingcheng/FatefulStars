#ifndef __ANIM_H__
#define __ANIM_H__

class WelcomeAnimation {
public:
    static const int DURATION = 180;
    static const int ANIMATION = 60;

    WelcomeAnimation(Game *game);
    ~WelcomeAnimation();

    void Init();
    void Update();
    void Draw();
    bool ShouldRender();

private:
    Game *game;
    Texture2D texture;
};

#endif
