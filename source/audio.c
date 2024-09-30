#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

typedef struct
{
    u8* buffer;
    u32 size;

    u32 flags;
	u32 channel;
} sound;

void loadSound(char* path, sound* soundPtr)
{
	FILE* file = fopen(path, "rb");
	fseek(file, 0, SEEK_END);
	soundPtr->size = ftell(file);
	fseek(file, 0, SEEK_SET);

	soundPtr->buffer = linearAlloc(soundPtr->size);
    fread(soundPtr->buffer, 1, soundPtr->size, file);

	fclose(file);
}

void stopSound(sound* soundPtr)
{
	csndExecCmds(true);
	CSND_SetPlayState(soundPtr->channel, 0);
	csndExecCmds(true);
}

void playSound(sound* soundPtr)
{
	stopSound(soundPtr);

	csndPlaySound(soundPtr->channel, soundPtr->flags, 48000, 1, 0, soundPtr->buffer, soundPtr->buffer, soundPtr->size);
}

void freeSound(sound* soundPtr)
{
	stopSound(soundPtr);

	memset(soundPtr->buffer, 0, soundPtr->size);
	GSPGPU_FlushDataCache(soundPtr->buffer, soundPtr->size);
	linearFree(soundPtr->buffer);
	soundPtr->buffer = NULL;
}