#ifndef __PLANET_H__
#define __PLANET_H__

class Planet {
public:
    Planet(Game* game);
    ~Planet();

    void Update();
    void Draw();
    PhysicsBody GetBody(void) {
        return body;
    }
    int GetID(void);

private:
    Game* game;
    PhysicsBody body;
    int x_position;
    int y_position;
    int radius;
    int dense;
};

#endif
