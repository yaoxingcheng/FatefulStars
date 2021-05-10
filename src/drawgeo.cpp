#include "game.h"
#include "drawgeo.h"
#include "raylib.h"
#include "physac.h"
#include <string>
using namespace std;

#define pi 3.141592653

void Pic::Init(string s, Vector2 imh, float imR, float ang, Color col) {
    imageheart = imh;
    imageR = imR;
    angular = ang;
    color = col;

    const int orgsize = 325;
    image = LoadImage(s.c_str());
    ImageCrop(&image,{0, 0, orgsize, orgsize});

    textureR = 30;
    float rat = textureR/imageR;
    ImageResize(&image,(int)(rat*orgsize),(int)(rat*orgsize));
    texture = LoadTextureFromImage(image);
    textureheart = {imageheart.x*rat, imageheart.y*rat};
}

void Pic::Print(Vector2 centroid, float rotation) {
    rotation += angular;
    float texturebias = rotation + (atan2(textureheart.y, textureheart.x)/pi*180);
    float r = sqrt(pow(textureheart.x,2) + pow(textureheart.y,2));
    DrawTextureEx(texture,{centroid.x - r * (float)cos(texturebias/180*pi), centroid.y - r * (float)sin(texturebias/180*pi)},rotation,1,color);
}

DrawGeo::DrawGeo() {
    Color mygreen = CLITERAL(Color){ 203, 255, 243, 255 };
    Color myred   = CLITERAL(Color){ 255, 198, 188, 255 };
    Color myyellow = CLITERAL(Color){ 255, 231, 169, 255 };
    Color myorange = CLITERAL(Color){ 254, 182, 140, 255 };
    pic[0].Init("resources/tmp-cir.png",{164,163},118,0,mygreen);
    pic[3].Init("resources/tmp-tra.png",{176,194},157,0,myred);
    pic[4].Init("resources/tmp-rec.png",{167,157},138,45,myyellow);
    pic[5].Init("resources/tmp-pen.png",{164,160},112,0,myorange);
    // CLITERAL(Color){ 255, 255, 255, 255 }
}

DrawGeo::~DrawGeo() {

}

void DrawGeo::DrawPolygon(int len, Vector2 *vectors) {
    // return;
    Vector2 o = {0.,0.};
    for (int i = 0; i < len; ++ i){
        o.x += vectors[i].x / len;
        o.y += vectors[i].y / len;
    }
    Vector2 v = {vectors[0].x - o.x, vectors[0].y - o.y};
    float rot = atan2(v.y,v.x)/pi*180 + 90;
    int who = len >= 6? 0: len;
    pic[who].Print(o,rot);
}
