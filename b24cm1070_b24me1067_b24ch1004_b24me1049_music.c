#include "raylib.h"
#include "music.h"
#include <stdlib.h>
#include <time.h>

#define MAX_TRACK 2

const char *track[MAX_TRACK] = {
    "resources/game_music.ogg",
    "resources/game_music2.ogg"
};

Music PlayRandomMusic() {
    srand(time(NULL));
    
    int index = rand() % MAX_TRACK;
    Music music = LoadMusicStream(track[index]);
    PlayMusicStream(music);
    return music;
}
Music PlayMenuMusic() {
    const char *menuTrack = "resources/menu_music.ogg";
    Music music = LoadMusicStream(menuTrack);
    PlayMusicStream(music);
    return music;
}
