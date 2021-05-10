#ifndef __DRAWGEO_H__
#define __DRAWGEO_H__

#include "physac.h"
#include <string>
using std::string;

class Pic {
public:
    void Init(string s, Vector2 imh, float imR, float ang, Color color);
    void Print(Vector2 centroid, float rotation);

private:
    Image image;
    Texture2D texture;
    Vector2 imageheart, textureheart;
    float imageR, textureR, angular;
    Color color;
};

class DrawGeo {
public:
    DrawGeo();
    ~DrawGeo();

    void DrawPolygon(int counts, Vector2 *vectors);

private:
    Pic pic[10];
};

#endif