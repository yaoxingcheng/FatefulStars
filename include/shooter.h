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

    void SetCursorX(int);
    void SetNextSides(int);
    void SetEnergy(int);
    void SetReleased(int);
    void Update();
    void Draw();
    int GetNextSides(){
        return next_sides;
    }

private:
    Game* game;
    ShooterPosition pos;
    float ball_radius;
    float ball_dense;
    PhysicsBody shooter_body;
    PhysicsBody holded_body;
    float getX();
    float getY();
    void createNewBody();
    void destroyBody();
    void drawBody();
    void drawShooter();
    void drawText();
    Vector2 getHoldPosition();
    int cursorX;
    int energy;
    int released;
    int next_sides;
};

#endif
