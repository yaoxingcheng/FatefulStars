#ifndef __PLANET_H__
#define __PLANET_H__

class Planet {
public:
    static const int SCENE_ANIMATION_FRAMES = 60;
    static const int SCENE_CHRACATER_FRAMES = 3;
    static const int SCENE_PARAGRAPH_INTERVAL_FRAMES = 120;
    static const int SCENE_FONT_SIZE = 20;

    Planet(Game* game);
    ~Planet();

    void Init();
    void Update();
    void Draw();
    void DrawBall();
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
    Texture2D bg;
    Texture2D* balls;
    int stage = 0;
    int scene_id = 0;
    int scene_time = 0;
    int scene_cost = 0;
    int scene_cur = 0;
    int scene_cd = 0;
    int scene_part = 0;
    int scene_line = 0;
    int scene_pos = 0;

    char* story[6];
    char disp[2][4][50];
    int char_num[6];
    int para_num[6];

    void nextScene();
    int calcSceneCost(int id);
};

#endif
