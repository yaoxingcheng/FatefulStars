#ifndef __SHOOTER_H__
#define __SHOOTER_H__

enum ShooterPosition {
    UP,
    DOWN,
};

class Shooter {
public:
    Shooter(Game* game, ShooterPosition pos, float ball_radius, float ball_dense);
    ~Shooter();

    void Update();
    void Draw();

private:
    Game* game;
    ShooterPosition pos;
    float ball_radius;
    float ball_dense;
    int holded_body_id;
    float getX();
    float getY();
    void createNewBody();
    void destroyBody();
    void drawBody();
    Vector2 getHoldPosition();
};

#endif
