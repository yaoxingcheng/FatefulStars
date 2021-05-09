#include "game.h"

int
main(void)
{
    Game *g = new Game();
    InitWindow(g->screenWidth, g->screenHeight, "sample game: snake");
    g->InitGame();

#if defined(PLATFORM_WEB)

    //emscripten_set_main_loop(UpdateDrawFrame, 60, 1);

#else

    SetTargetFPS(60);

    // InitAudioDevice();
    // Music must = LoadMusicStream("resour/ct.mp3"); 
    // PlayMusicStream(must);
    InitAudioDevice();              // Initialize audio device
    // Wave wave = LoadWave("resour/mc.mp3");
    // Sound sound = LoadSoundFromWave(wave);
    Sound sound = LoadSound("resour/posui.mp3");
    PlaySoundMulti(sound);
    // Music music = LoadMusicStream("resour/posui.mp3");
    // PlayMusicStream(music);
    Music music2 = LoadMusicStream("resour/zhuangji.mp3");
    PlayMusicStream(music2);

    //Main game loop
    //Detect window close button or ESC key
    while (!WindowShouldClose()) 
    {
        // if (!IsSoundPlaying(sound)) PlaySound(sound);
        static int i=0;
        if (++i %10 ==0 ) PlaySoundMulti(sound);
        // UpdateMusicStream(music);
        // UpdateMusicStream(music2);
        g->UpdateGame();
        g->DrawGame();
    }

#endif

    //Close window and OpenGL context
    CloseWindow();

    return 0;
}
