#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

typedef struct
{
    char* file;
    u8* buffer;
    u32 size;
    u32 flags;
} sound;

sound jumpSfx = { "romfs:/jump.bin", NULL, 0, SOUND_FORMAT_16BIT | SOUND_ONE_SHOT };
sound scoreSfx = { "romfs:/score.bin", NULL, 0, SOUND_FORMAT_16BIT | SOUND_ONE_SHOT };

void audioPlay(sound* sound)
{
	FILE* file = fopen(sound->file, "rb");
	fseek(file, 0, SEEK_END);
	sound->size = ftell(file);
	fseek(file, 0, SEEK_SET);

	sound->buffer = linearAlloc(sound->size);
    fread(sound->buffer, 1, sound->size, file);

	fclose(file);

	csndPlaySound(8, sound->flags, 48000, 1, 0, sound->buffer, sound->buffer, sound->size);

	linearFree(sound->buffer);
}

void audioStop(sound* sound)
{
	csndExecCmds(true);
	CSND_SetPlayState(0x8, 0);
	memset(sound->buffer, 0, sound->size);
	GSPGPU_FlushDataCache(sound->buffer, sound->size);
	linearFree(sound->buffer);
}