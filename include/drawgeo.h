#ifndef __DRAWGEO_H__
#define __DRAWGEO_H__

#include "physac.h"

class DrawGeo {
public:
    DrawGeo();
    ~DrawGeo();

    void DrawPolygon(int len, Vector2 *vectors);

private:
    Image images[10];
    Texture2D textures[10];
    Vector2 imheart[10], teheart[10];
    float imR[10], teR, angulars[10];
};

#endif
