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
    int GetID(void) {
        return body_id;
    }

private:
    Game* game;
    PhysicsBody body;
    int body_id;
    int x_position;
    int y_position;
    int radius;
    int dense;
};

#endif
