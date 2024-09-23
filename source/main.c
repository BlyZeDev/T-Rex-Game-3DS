#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

#include "sprites.h"

#define TOP_SCREEN_WIDTH  400
#define BOTTOM_SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

int main(int argc, char **argv)
{
    romfsInit();
	gfxInitDefault();
    hidInit();
	mcuHwcInit();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

    C3D_RenderTarget* topPtr = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

    C2D_SpriteSheet spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
    if (!spriteSheet) svcBreak(USERBREAK_PANIC);

    size_t spriteCount = 0;
    sprite_Anim* sprites = initSprites(spriteSheet, &spriteCount);

    sprite_Anim* curSpritePtr;

    u32 frames = 60;
	while (aptMainLoop())
	{
        hidScanInput();

        if (hidKeysDown() & KEY_START) break;

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        
        frames++;

        C2D_TargetClear(topPtr, C2D_Color32(0, 0, 0, 255));
		C2D_SceneBegin(topPtr);
        
        for (size_t i = 0; i < spriteCount; i++)
        {
            curSpritePtr = &sprites[i];

            if (curSpritePtr->fps > 0)
            {
                if (frames % curSpritePtr->fps == 0)
                {
                    curSpritePtr->curIndex++;

                    if (curSpritePtr->curIndex > curSpritePtr->numFrames - 1) curSpritePtr->curIndex = 0;
                }
            }

            C2D_DrawSprite(&curSpritePtr->frames[curSpritePtr->curIndex]);
        }
        
        C3D_FrameEnd(0);
	}

    C2D_Fini();
    C3D_Fini();
	mcuHwcExit();
	hidExit();
	gfxExit();
    romfsExit();
	return 0;
}