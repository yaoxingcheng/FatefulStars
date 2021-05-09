#ifndef __PLANET_H__
#define __PLANET_H__

class Planet {
public:
    Planet(Game* game);
    ~Planet();

    void Update();
    void Draw();

private:
    Game* game;

};

#endif
