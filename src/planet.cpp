#include <algorithm>
#include <cctype>
#include "game.h"
#include "planet.h"
#include "anim.h"
#include <cstring>

Planet::Planet(Game* game) : game(game) {
    int L = std::min(game->screenWidth, game->screenHeight);
    x_position = L / 2;
    y_position = L / 2;
    radius = 110;
    dense = 100;
    body = CreatePhysicsBodyCircle((Vector2){float(x_position), float(y_position)}, radius, dense);
    body->enabled = false;
}

Planet::~Planet() {
    UnloadTexture(bg);
    for (int i = 0; i < 4; ++i) {
        UnloadTexture(balls[i]);
    }
    delete[] balls;
}

void Planet::Init() {
    bg = LoadTexture("resources/bg.png");
    balls = new Texture2D[4];
    for (int i = 0; i < 4; ++i) {
        balls[i] = LoadTexture(TextFormat("resources/ball%d.png", i));
    }
    char* texts = LoadFileText("story/story.txt");

    int charCnt = 0;
    int paraCnt = 1;
    int sceneCnt = 0;
    int cur = 0;
    story[0] = &texts[0];
    while (texts[cur] != 0) {
        char c = texts[cur];
        if (c == '#') {
            texts[cur] = 0;
            char_num[sceneCnt] = charCnt;
            para_num[sceneCnt] = paraCnt;
            story[++sceneCnt] = &texts[cur + 1];
            charCnt = 0;
            paraCnt = 1;
        } else if (c == '-') {
            paraCnt ++;
        } else if (!isblank(c)) {
            charCnt ++;
        }
        cur ++;
    }
    scene_id = 0;
    scene_time = 0;
    scene_cost = calcSceneCost(0);
    scene_cur = 0;
    scene_cd = 0;
    scene_part = 0;
    scene_line = 0;
    scene_pos = 0;
    memset(disp, 0, sizeof(disp));
}

int Planet::calcSceneCost(int id) {
    return SCENE_ANIMATION_FRAMES * 2 + SCENE_CHRACATER_FRAMES * char_num[id] + SCENE_PARAGRAPH_INTERVAL_FRAMES * (para_num[id] - 1);
}

int Planet::GetID() {
    if (body == NULL) return -1;
    return body->id;
}

void Planet::nextScene() {
    if (scene_time < 0 && scene_id < 4) {
        scene_id += 1;
        scene_time = 0;
        scene_cost = calcSceneCost(scene_id);
        scene_cur = 0;
        scene_cd = 0;
        scene_part = 0;
        scene_line = 0;
        scene_pos = 0;
        memset(disp, 0, sizeof(disp));
    }
}

void Planet::Update() {
    int score = game->GetScore();
    if (stage == 0 && score >= 5) {
        nextScene();
        stage = 1;
    } else if (stage == 1 && score >= 10) {
        nextScene();
        stage = 2;
    } else if (stage == 2 && score >= 20) {
        nextScene();
        stage = 3;
    } else if (stage == 3 && score >= 30) {
        nextScene();
    }
}

void Planet::Draw() {
    const int DURATION = WelcomeAnimation::DURATION;
    const int ANIMATION = WelcomeAnimation::ANIMATION;
    DrawTexture(bg, 0, 0, Fade(WHITE, 1.0f * std::min(game->framesCounter - DURATION - ANIMATION / 2, ANIMATION) / ANIMATION));
}

void Planet::DrawBall() {
    const int DURATION = WelcomeAnimation::DURATION;
    const int ANIMATION = WelcomeAnimation::ANIMATION;
    DrawTexture(balls[stage], 0, 0, Fade(WHITE, 1.0f * std::min(game->framesCounter - DURATION - ANIMATION / 2, ANIMATION) / ANIMATION));

    if (scene_time >= 0) {
        float alpha = 0.72;
        if (scene_time < SCENE_ANIMATION_FRAMES) {
            alpha = alpha * scene_time / SCENE_ANIMATION_FRAMES;
        } else if (scene_cost - scene_time < SCENE_ANIMATION_FRAMES) {
            alpha = alpha * (scene_cost - scene_time) / SCENE_ANIMATION_FRAMES;
        }
        DrawRectangle(0, 0, game->screenWidth, game->screenHeight, Fade(BLACK, alpha));
        if (scene_cd == 0) {
            char c;
            if (scene_line == 0 && scene_pos == 0) {
                memset(disp[scene_part], 0, sizeof(disp[scene_part]));
            }
            while ((c = story[scene_id][scene_cur]) != 0) {
                scene_cur ++;
                if (c == '-') {
                    scene_part ^= 1;
                    scene_line = 0;
                    scene_pos = 0;
                    scene_cd = SCENE_PARAGRAPH_INTERVAL_FRAMES;
                    break;
                } else if (c == '\n' || c == '\r') {
                    scene_line ++;
                    scene_pos = 0;
                    continue;
                }
                disp[scene_part][scene_line][scene_pos ++] = c;
                disp[scene_part][scene_line][scene_pos] = 0;
                if (!isblank(c)) break;
            }
        }
        if (!(scene_cost - scene_time <= SCENE_ANIMATION_FRAMES / 2)) {
            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 3; ++j) {
                    DrawText(disp[i][j], 200, 150 + i * 400 + j * 36, SCENE_FONT_SIZE, WHITE);
                }
            }
        }
        if (scene_cd == 0) {
            scene_cd = SCENE_CHRACATER_FRAMES - 1;
        } else {
            scene_cd --;
        }
        if (scene_time >= scene_cost) {
            scene_time = -1;
        } else {
            scene_time ++;
        }
    }
}
