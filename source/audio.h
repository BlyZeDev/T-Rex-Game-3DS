#ifndef AUDIO_H
#define AUDIO_H

typedef struct
{
    u8* buffer;
    u32 size;

    u32 flags;
	u32 channel;
} sound;

void loadSound(char* path, sound* soundPtr);

void stopSound(sound* soundPtr);

void playSound(sound* soundPtr);

void freeSound(sound* soundPtr);

#endif