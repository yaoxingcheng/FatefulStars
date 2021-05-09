#include "game.h"
#include "drawgeo.h"

DrawGeo::DrawGeo() {
    const int orgsize = 325;

    images[0] = LoadImage("resources/tmp-cir.png");
    ImageCrop(&images[0],{0, 0, orgsize, orgsize});
    imheart[0] = {165, 172}; imR[0] = 118; angulars[0] = 0;

    images[3] = LoadImage("resources/tmp-tra.png");
    ImageCrop(&images[3],{0, 0, orgsize, orgsize});
    imheart[3] = {176, 194}; imR[3] = 157; angulars[3] = 0;

    images[4] = LoadImage("resources/tmp-rec.png");
    ImageCrop(&images[4],{0, 0, orgsize, orgsize});
    imheart[4] = {167, 157}; imR[4] = 138; angulars[4] = 45;

    teR = 30;
    // printf("fweaonfoawiefnweaif\n\n\n\n\n\n\n\n\n"); fflush(stdout);
    for (int i = 0; i < 10; ++i){
        if (imR[i]<0.5) continue;
        float rat = teR/imR[i];
        ImageResize(&images[i],(int)(rat*orgsize),(int)(rat*orgsize));
        textures[i] = LoadTextureFromImage(images[i]);
        teheart[i] = {imheart[i].x*rat, imheart[i].y*rat};
    }
    // printf("oewigjnoerfnafdes\n\n\n\n\n\n\n\n\n"); fflush(stdout);
}

DrawGeo::~DrawGeo() {

}

void DrawGeo::DrawPolygon(int len, Vector2 *vectors) {
    // return;
    Vector2 o = {0.,0.};
    for (int i=0;i<len;++i){
        o.x+=vectors[i].x/len;
        o.y+=vectors[i].y/len;
    }
    if (len == 6||1) {
        int i = 0;
        // float rot = 
        DrawTextureEx(textures[i],{o.x-teheart[i].x,o.y-teheart[i].y},0,1,CLITERAL(Color){ 255, 255, 255, 255 });
    }
}

