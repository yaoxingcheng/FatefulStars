#ifndef __SHOOTER_H__
#define __SHOOTER_H__

#include "drawgeo.h"

enum ShooterPosition {
    UP,
    DOWN,
};

class Shooter {
public:
    Shooter(Game* game, ShooterPosition pos, float ball_radius, float ball_dense);
    ~Shooter();

    void SetCursorX(int);
    void SetNumSides(int);
    void SetNumShot(int);
    void SetEnergy(int);
    void SetReleased(int);
    void Update();
    void Draw();
    void Init();
    int GetNumSides(){
        return num_sides;
    }
    int GetNumShot(){
        return num_shot;
    }
    int GetCurrentShot(){
        return current_shot;
    }
    int GetEnergy(){
        return energy;
    }

private:
    Game* game;
    DrawGeo* drawgeo;
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
    Texture2D texture;
    int energy;
    int released;
    int num_shot;
    int num_sides;
    int current_shot;
    int score = 0;
};

#endif
