#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

#include "game.h"

#define TOP_SCREEN_WIDTH  400
#define BOTTOM_SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

void init()
{
    romfsInit();
	gfxInitDefault();
    hidInit();
	mcuHwcInit();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
}

void close()
{
    C2D_Fini();
    C3D_Fini();
	mcuHwcExit();
	hidExit();
	gfxExit();
    romfsExit();
}

void updateGround(sprite* ground, sprite* groundExt)
{
    if (ground->frames[groundExt->curIndex].params.pos.x < -ground->hitbox.width)
    {
        setSpritePos(ground, groundExt->frames[groundExt->curIndex].params.pos.x + groundExt->hitbox.width, groundExt->frames[0].params.pos.y);
    }
    else if (groundExt->frames[groundExt->curIndex].params.pos.x < -groundExt->hitbox.width)
    {
        setSpritePos(groundExt, ground->frames[groundExt->curIndex].params.pos.x + ground->hitbox.width, ground->frames[0].params.pos.y);
    }

    moveSprite(ground, -5, 0);
    moveSprite(groundExt, -5, 0);
}

void updatePlayer(player* player, u32 kDown, u32 kHeld, u32 kUp)
{

}

int main(int argc, char **argv)
{
    init();

    C3D_RenderTarget* topPtr = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

    C2D_SpriteSheet spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
    if (!spriteSheet) svcBreak(USERBREAK_PANIC);

    sprite ground = initGround(spriteSheet);
    setSpritePos(&ground, 0, SCREEN_HEIGHT - ground.hitbox.height);

    sprite groundExt = initGround(spriteSheet);
    setSpritePos(&groundExt, ground.hitbox.width, SCREEN_HEIGHT - groundExt.hitbox.height);

    sprite clound = initCloud(spriteSheet);
    sprite sun = initSun(spriteSheet);
    sprite gameOver = initGameOver(spriteSheet);

    player player = initPlayer(spriteSheet);
    setPlayerPos(&player, 10, SCREEN_HEIGHT - player.sprites[player.state].hitbox.height - ground.hitbox.height / 2);

    sprite* cacti = initCacti(spriteSheet);
    sprite bird = initBird(spriteSheet);

    u32 frames = 0;

    u32 hKeysDown;
    u32 hKeysHeld;
    u32 hKeysUp;
	while (aptMainLoop())
	{
        hidScanInput();

        hKeysDown = hidKeysDown();
        hKeysHeld = hidKeysHeld();
        hKeysUp = hidKeysUp();

        if (hKeysDown & KEY_START) break;

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        
        frames++;

        C2D_TargetClear(topPtr, C2D_Color32(0, 0, 0, 255));
		C2D_SceneBegin(topPtr);

        updateGround(&ground, &groundExt);
        updatePlayer(&player, hKeysDown, hKeysHeld, hKeysUp);

        renderSprite(&ground, frames);
        renderSprite(&groundExt, frames);

        renderSprite(&player.sprites[player.curIndex], frames);
        
        C3D_FrameEnd(0);
	}

    C2D_SpriteSheetFree(spriteSheet);

    close();
	return 0;
}