#ifndef AUDIO_H
#define AUDIO_H

typedef struct
{
    char* file;
    u8* buffer;
    u32 size;
    u32 flags;
} sound;

extern sound jumpSfx;
extern sound scoreSfx;

void audioPlay(sound* sound);

void audioStop(sound* sound);

#endif