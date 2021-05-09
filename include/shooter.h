#ifndef __SHOOTER_H__
#define __SHOOTER_H__

enum ShooterPosition {
    UP,
    DOWN,
};

class Shooter {
public:
    Shooter(Game* game, ShooterPosition pos);
    ~Shooter();

    void SetCursorX(int);
    void Update();
    void Draw();

private:
    Game* game;
    ShooterPosition pos;
    int cursorX;
};

#endif
